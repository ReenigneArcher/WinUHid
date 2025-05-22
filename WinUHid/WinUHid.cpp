#include "pch.h"
#include "WinUHid.h"

#include <wrl/wrappers/corewrappers.h>
using namespace Microsoft::WRL;

//
// IOCTL interfaces for WinUHid
//
#include "../WinUHid Driver/Public.h"

enum class WINUHID_DEVICE_STATE {
	Created,
	Started,
	Stopped,
};

typedef struct _WINUHID_DEVICE {
	//
	// Handle to the WinUHid file object for this device
	//
	HANDLE Handle;

	//
	// Lock to guard state changes and the event size hint
	//
	SRWLOCK Lock;

	//
	// Current state of the device
	//
	WINUHID_DEVICE_STATE State;

	//
	// Hint of how large of a WINUHID_EVENT buffer we should allocate when polling
	//
	DWORD EventBufferSizeHint;

	//
	// Async event processing for devices which are started with a callback
	//
	HANDLE EventThread;
	PWINUHID_EVENT_CALLBACK EventCallback;
	PVOID CallbackContext;
} WINUHID_DEVICE, *PWINUHID_DEVICE;

size_t MultiSzWcsLen(PCWSTR MultiSzString)
{
	size_t len = 0;

	while (*MultiSzString != UNICODE_NULL) {
		len += 1 + wcslen(MultiSzString);
		MultiSzString += len;
	}

	return len;
}

BOOL DeviceIoControlInSync(HANDLE Handle, HANDLE OverlappedEvent, DWORD Ioctl, LPCVOID InBuffer, DWORD InBufferSize)
{
	OVERLAPPED overlapped = {};
	Wrappers::Event privateEvent;
	DWORD bytesRead;
	BOOL ret;

	//
	// Use the caller's event if provided, otherwise create one for this operation
	//
	if (OverlappedEvent) {
		ResetEvent(OverlappedEvent);
		overlapped.hEvent = OverlappedEvent;
	}
	else {
		privateEvent.Attach(CreateEventW(NULL, TRUE, FALSE, NULL));
		if (!privateEvent.IsValid()) {
			return FALSE;
		}

		overlapped.hEvent = privateEvent.Get();
	}

	ret = DeviceIoControl(Handle, Ioctl, const_cast<LPVOID>(InBuffer), InBufferSize, NULL, 0, &bytesRead, &overlapped);
	if (!ret && GetLastError() == ERROR_IO_PENDING) {
		ret = GetOverlappedResultEx(Handle, &overlapped, &bytesRead, INFINITE, FALSE);
	}

	return ret;
}

WINUHID_API DWORD WinUHidGetDriverInterfaceVersion()
{
	DWORD version;
	DWORD bytesReturned;

	//
	// We don't technically need administrator permissions for this,
	// but we request GENERIC_READ|GENERIC_WRITE to ensure this is
	// a good indicator of whether the driver can actually be used.
	//
	Wrappers::FileHandle handle{ CreateFileW(WINUHID_WIN32_PATH,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL,
		OPEN_EXISTING,
		0,
		NULL) };
	if (!handle.IsValid()) {
		return 0;
	}

	if (!DeviceIoControl(handle.Get(), IOCTL_WINUHID_GET_INTERFACE_VERSION, NULL, 0, &version, sizeof(version), &bytesReturned, NULL)) {
		version = 0;
	}

	return version;
}

WINUHID_API PWINUHID_DEVICE WinUHidCreateDevice(PCWINUHID_DEVICE_CONFIG Config)
{
	PWINUHID_DEVICE device;
	WINUHID_DEVICE_INFO devInfo;

	if (Config == NULL || Config->ReportDescriptor == NULL || Config->ReportDescriptorLength == 0) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return NULL;
	}

	Wrappers::Event overlappedEvent{ CreateEventW(NULL, TRUE, FALSE, NULL) };
	if (!overlappedEvent.IsValid()) {
		return NULL;
	}

	device = (PWINUHID_DEVICE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*device));
	if (!device) {
		SetLastError(ERROR_OUTOFMEMORY);
		goto Fail;
	}

	InitializeSRWLock(&device->Lock);
	device->State = WINUHID_DEVICE_STATE::Created;
	device->EventBufferSizeHint = 128;

	device->Handle = CreateFileW(WINUHID_WIN32_PATH,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED,
		NULL);
	if (device->Handle == INVALID_HANDLE_VALUE) {
		goto Fail;
	}

	//
	// Set base device information
	//
	devInfo.InterfaceVersion = WINUHID_VERSION_LATEST;
	devInfo.SupportedEvents = Config->SupportedEvents;
	devInfo.VendorID = Config->VendorID;
	devInfo.ProductID = Config->ProductID;
	devInfo.VersionNumber = Config->VersionNumber;
	devInfo.ContainerId = Config->ContainerId;
	if (!DeviceIoControlInSync(device->Handle, overlappedEvent.Get(), IOCTL_WINUHID_SET_DEVICE_INFO, &devInfo, sizeof(devInfo))) {
		goto Fail;
	}

	//
	// Set the required report descriptor
	//
	if (!DeviceIoControlInSync(device->Handle, overlappedEvent.Get(), IOCTL_WINUHID_SET_REPORT_DESCRIPTOR, Config->ReportDescriptor, Config->ReportDescriptorLength)) {
		goto Fail;
	}

	//
	// Set the instance ID if one was provided
	//
	if (Config->InstanceID != NULL) {
		if (!DeviceIoControlInSync(device->Handle, overlappedEvent.Get(), IOCTL_WINUHID_SET_INSTANCE_ID, (LPVOID)Config->InstanceID, (DWORD)(wcslen(Config->InstanceID) + 1) * sizeof(WCHAR))) {
			goto Fail;
		}
	}

	//
	// Set additional hardware IDs if provided
	//
	if (Config->HardwareIDs != NULL) {
		if (!DeviceIoControlInSync(device->Handle, overlappedEvent.Get(), IOCTL_WINUHID_SET_HARDWARE_IDS, (LPVOID)Config->HardwareIDs, (DWORD)(MultiSzWcsLen(Config->HardwareIDs) + 1) * sizeof(WCHAR))) {
			goto Fail;
		}
	}

	//
	// Finally, create the device
	//
	if (!DeviceIoControlInSync(device->Handle, overlappedEvent.Get(), IOCTL_WINUHID_CREATE_DEVICE, NULL, 0)) {
		goto Fail;
	}

	return device;

Fail:
	WinUHidDestroyDevice(device);
	return NULL;
}

WINUHID_API BOOL WinUHidSubmitInputReport(PWINUHID_DEVICE Device, LPCVOID Report, DWORD ReportSize)
{
	OVERLAPPED overlapped = {};
	DWORD bytesWritten;
	BOOL ret;

	if (Device == NULL || Report == NULL || ReportSize == 0) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	Wrappers::Event overlappedEvent{ CreateEventW(NULL, TRUE, FALSE, NULL) };
	if (!overlappedEvent.IsValid()) {
		return FALSE;
	}

	overlapped.hEvent = overlappedEvent.Get();

	ret = WriteFile(Device->Handle, Report, ReportSize, &bytesWritten, &overlapped);
	if (!ret && GetLastError() == ERROR_IO_PENDING) {
		ret = GetOverlappedResultEx(Device->Handle, &overlapped, &bytesWritten, INFINITE, FALSE);
	}

	return ret;
}

DWORD WINAPI EventThreadProc(LPVOID lpParameter)
{
	PWINUHID_DEVICE device = (PWINUHID_DEVICE)lpParameter;
	PCWINUHID_EVENT event;

	while ((event = WinUHidPollEvent(device, INFINITE))) {
		device->EventCallback(device->CallbackContext, device, event);
	}

	return GetLastError();
}

WINUHID_API BOOL WinUHidStartDevice(PWINUHID_DEVICE Device, PWINUHID_EVENT_CALLBACK EventCallback, PVOID CallbackContext)
{
	if (Device == NULL) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	//
	// Acquire the device lock exclusively as we're about to mutate state
	//
	AcquireSRWLockExclusive(&Device->Lock);

	//
	// This is only allowed in the Created state
	//
	if (Device->State != WINUHID_DEVICE_STATE::Created) {
		ReleaseSRWLockExclusive(&Device->Lock);
		SetLastError(ERROR_BAD_COMMAND);
		return FALSE;
	}

	//
	// If a callback was provided, we will start the event thread
	//
	Device->EventCallback = EventCallback;
	Device->CallbackContext = CallbackContext;
	if (Device->EventCallback != NULL) {
		Device->EventThread = CreateThread(NULL, 0, EventThreadProc, Device, 0, NULL);
		if (Device->EventThread == NULL) {
			ReleaseSRWLockExclusive(&Device->Lock);
			return FALSE;
		}

		//
		// Raise the priority of the event thread to ensure it can quickly service
		// pending HID requests and then go back to sleep. The scheduling latency
		// of this thread directly determines the performance of the HID client
		// reading/writing to this device on the other side.
		//
		// Additionally, if this thread is starved too long, VHF may decide that
		// we're out to lunch and silently cancel our pending requests for us.
		//
		SetThreadPriority(Device->EventThread, THREAD_PRIORITY_TIME_CRITICAL);
	}

	if (!DeviceIoControlInSync(Device->Handle, NULL, IOCTL_WINUHID_START_DEVICE, NULL, 0)) {
		//
		// Unwinding the event thread creation is relatively simple. We are not yet
		// in the Started state and we hold the device lock exclusively, so the
		// thread will be blocking inside WinUHidPollEvent() on the lock. When we
		// release the lock, it will observe that we're not in a valid state to poll
		// and fail, causing the thread to exit.
		//
		ReleaseSRWLockExclusive(&Device->Lock);
		if (Device->EventThread != NULL) {
			WaitForSingleObject(Device->EventThread, INFINITE);
			CloseHandle(Device->EventThread);
			Device->EventThread = NULL;
		}
		return FALSE;
	}

	//
	// We are now ready to start processing operations!
	//
	Device->State = WINUHID_DEVICE_STATE::Started;

	ReleaseSRWLockExclusive(&Device->Lock);

	return TRUE;
}

WINUHID_API PCWINUHID_EVENT WinUHidPollEvent(PWINUHID_DEVICE Device, DWORD TimeoutMillis)
{
	PWINUHID_EVENT event;
	DWORD bufferSize;

	if (Device == NULL) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return NULL;
	}

	Wrappers::Event overlappedEvent{ CreateEventW(NULL, TRUE, FALSE, NULL) };
	if (!overlappedEvent.IsValid()) {
		return NULL;
	}

	//
	// Acquire the device lock in shared mode to protect the state and event buffer size hints
	//
	AcquireSRWLockShared(&Device->Lock);
	bufferSize = Device->EventBufferSizeHint;
	event = (PWINUHID_EVENT)HeapAlloc(GetProcessHeap(), 0, bufferSize);
	if (event == NULL) {
		ReleaseSRWLockShared(&Device->Lock);
		SetLastError(ERROR_OUTOFMEMORY);
		return NULL;
	}
	for (;;) {
		BOOL ret;
		DWORD bytesWritten;
		PWINUHID_EVENT newEvent;
		OVERLAPPED overlapped;

		//
		// If the device has been stopped, abort now
		//
		if (Device->State != WINUHID_DEVICE_STATE::Started) {
			ReleaseSRWLockShared(&Device->Lock);
			SetLastError(ERROR_BAD_COMMAND);
			goto Fail;
		}

		//
		// Issue the asynchronous IOCTL
		//
		RtlZeroMemory(&overlapped, sizeof(overlapped));
		overlapped.hEvent = overlappedEvent.Get();
		ResetEvent(overlapped.hEvent);
		ret = DeviceIoControl(Device->Handle, IOCTL_WINUHID_GET_NEXT_EVENT, NULL, 0, event, bufferSize, &bytesWritten, &overlapped);
		ReleaseSRWLockShared(&Device->Lock);

		if (!ret && GetLastError() == ERROR_IO_PENDING) {
			//
			// Wait for the IOCTL to complete or the timeout to expire
			//
			ret = GetOverlappedResultEx(Device->Handle, &overlapped, &bytesWritten, TimeoutMillis, FALSE);
			if (!ret && GetLastError() == WAIT_TIMEOUT) {
				//
				// If the timeout expired, we need to cancel the pending I/O and wait for completion.
				//
				// NB: We MUST wait here, even if it exceeds the caller's timeout, because we have
				// our OVERLAPPED struct on the stack and it will corrupt the stack if the I/O
				// completes after we've returned from this function.
				//
				// NB2: It's important to note here that cancellation is NOT guaranteed. If the IRP
				// has already been dequeued by the driver or is in the process of being completed,
				// we could receive a successful status code. If that happens, we must continue
				// processing the request as if it didn't time out.
				//
				CancelIoEx(Device->Handle, &overlapped);
				ret = GetOverlappedResultEx(Device->Handle, &overlapped, &bytesWritten, INFINITE, FALSE);
				if (!ret) {
					SetLastError(ERROR_TIMEOUT);
					goto Fail;
				}
			}
		}

		if (!ret && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
			//
			// If the buffer was too small, allocate a larger one and try again
			//
			bufferSize *= 2;
			AcquireSRWLockExclusive(&Device->Lock);
			if (Device->EventBufferSizeHint < bufferSize) {
				Device->EventBufferSizeHint = bufferSize;
			}
			else {
				bufferSize = Device->EventBufferSizeHint;
			}
			ReleaseSRWLockExclusive(&Device->Lock);

			newEvent = (PWINUHID_EVENT)HeapReAlloc(GetProcessHeap(), 0, event, bufferSize);
			if (newEvent == NULL) {
				SetLastError(ERROR_OUTOFMEMORY);
				goto Fail;
			}
			else {
				event = newEvent;
			}

			AcquireSRWLockShared(&Device->Lock);
			continue;
		}
		else if (!ret) {
			//
			// If any other error occurs, it's a failure
			//
			goto Fail;
		}
		else {
			//
			// Upon success, the event has been filled. However, if this is a read event,
			// we will allocate additional space after the event to contain the completion data.
			//
			if (WINUHID_EVENT_TYPE_IS_READ(event->Type) && bufferSize < bytesWritten + FIELD_OFFSET(WINUHID_READ_EVENT_COMPLETE, Data) + event->Read.DataLength) {
				bufferSize = bytesWritten + FIELD_OFFSET(WINUHID_READ_EVENT_COMPLETE, Data) + event->Read.DataLength;

				AcquireSRWLockExclusive(&Device->Lock);
				if (Device->EventBufferSizeHint < bufferSize) {
					Device->EventBufferSizeHint = bufferSize;
				}
				ReleaseSRWLockExclusive(&Device->Lock);

				newEvent = (PWINUHID_EVENT)HeapReAlloc(GetProcessHeap(), 0, event, bufferSize);
				if (newEvent == NULL) {
					//
					// Okay, this is fun. We have an event but can't allocate enough memory to return it.
					// We'll need to complete it here inline to avoid leaking the request.
					//
					WINUHID_READ_EVENT_COMPLETE readComplete;
					readComplete.RequestId = event->RequestId;
					readComplete.Status = STATUS_NO_MEMORY;
					readComplete.DataLength = 0;
					DeviceIoControlInSync(Device->Handle, overlappedEvent.Get(), IOCTL_WINUHID_COMPLETE_READ_EVENT, &readComplete, sizeof(readComplete));

					SetLastError(ERROR_OUTOFMEMORY);
					goto Fail;
				}
				else {
					event = newEvent;
				}
			}

			return event;
		}
	}

Fail:
	HeapFree(GetProcessHeap(), 0, event);
	return NULL;
}

WINUHID_API VOID WinUHidCompleteWriteEvent(PWINUHID_DEVICE Device, PCWINUHID_EVENT Event, BOOL Success)
{
	WINUHID_WRITE_EVENT_COMPLETE eventComplete;

	eventComplete.RequestId = Event->RequestId;
	eventComplete.Status = Success ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
	DeviceIoControlInSync(Device->Handle, NULL, IOCTL_WINUHID_COMPLETE_WRITE_EVENT, &eventComplete, sizeof(eventComplete));

	HeapFree(GetProcessHeap(), 0, const_cast<PWINUHID_EVENT>(Event));
}

WINUHID_API VOID WinUHidCompleteReadEvent(PWINUHID_DEVICE Device, PCWINUHID_EVENT Event, LPCVOID Data, DWORD DataLength)
{
	PWINUHID_READ_EVENT_COMPLETE eventComplete;

	//
	// To avoid having to allocate memory here (which could fail), we allocate extra memory in
	// WinUHidPollEvent() for the WINUHID_READ_EVENT_COMPLETE at the end of the WINUHID_EVENT.
	//
	eventComplete = (PWINUHID_READ_EVENT_COMPLETE)(Event + 1);

	eventComplete->RequestId = Event->RequestId;

	if (Data != NULL) {
		eventComplete->Status = STATUS_SUCCESS;

		//
		// If additional data is provided, truncate to the requested buffer size.
		//
		if (DataLength > Event->Read.DataLength) {
			eventComplete->DataLength = Event->Read.DataLength;
		}
		else {
			eventComplete->DataLength = DataLength;
		}

		RtlCopyMemory(&eventComplete->Data[0], Data, eventComplete->DataLength);
	}
	else {
		eventComplete->Status = STATUS_UNSUCCESSFUL;
		eventComplete->DataLength = 0;
	}

	DeviceIoControlInSync(Device->Handle, NULL, IOCTL_WINUHID_COMPLETE_READ_EVENT, eventComplete, FIELD_OFFSET(WINUHID_READ_EVENT_COMPLETE, Data) + eventComplete->DataLength);

	HeapFree(GetProcessHeap(), 0, const_cast<PWINUHID_EVENT>(Event));
}

WINUHID_API VOID WinUHidStopDevice(PWINUHID_DEVICE Device)
{
	if (Device == NULL) {
		return;
	}

	//
	// Acquire the device lock exclusively as we're about to mutate state
	//
	AcquireSRWLockExclusive(&Device->Lock);

	if (Device->State == WINUHID_DEVICE_STATE::Started) {
		//
		// First, change the device state to ensure no further requests will be dispatched
		//
		Device->State = WINUHID_DEVICE_STATE::Stopped;

		//
		// Next, cancel all pending operations on the device. This will cause
		// WinUHidPollEvent() to return if it's currently waiting in any threads.
		//
		CancelIoEx(Device->Handle, NULL);

		//
		// If we have an event thread, wait for it to exit now
		//
		if (Device->EventThread != NULL) {
			//
			// Drop the lock while waiting to ensure WinUHidPollEvent() can complete
			// and return if it was in the middle of I/O processing.
			//
			// NB: Since we already transitioned to Stopped, this is safe and we can
			// be sure no new IOCTLs will be submitted via WinUHidPollEvent().
			//
			ReleaseSRWLockExclusive(&Device->Lock);
			WaitForSingleObject(Device->EventThread, INFINITE);
			AcquireSRWLockExclusive(&Device->Lock);

			//
			// Finish cleaning up event thread state
			//
			CloseHandle(Device->EventThread);
			Device->EventThread = NULL;
		}
	}

	ReleaseSRWLockExclusive(&Device->Lock);
}

WINUHID_API VOID WinUHidDestroyDevice(PWINUHID_DEVICE Device)
{
	if (Device == NULL) {
		return;
	}

	//
	// The caller is responsible for ensuring WinUHidDestroyDevice() is only called
	// when no other threads are using the device, so we don't need the lock here.
	//
	if (Device->State == WINUHID_DEVICE_STATE::Started) {
		WinUHidStopDevice(Device);
	}

	if (Device->Handle != NULL && Device->Handle != INVALID_HANDLE_VALUE) {
		CloseHandle(Device->Handle);
	}

	HeapFree(GetProcessHeap(), 0, Device);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}