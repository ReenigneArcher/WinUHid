#include "pch.h"
#include "Utilities.h"
#include <thread>

TEST(HIDLL, DriverInterfaceVersion) {
	EXPECT_EQ(WinUHidGetDriverInterfaceVersion(), 1);
}

TEST(HIDLL, InputReport) {
	static const UCHAR reportDescriptor[] =
	{
		0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
		0x09, 0x05,        // Usage (Game Pad)
		0xA1, 0x01,        // Collection (Application)
		0x09, 0x30,        //   Usage (X)
		0x09, 0x31,        //   Usage (Y)
		0x09, 0x32,        //   Usage (Z)
		0x09, 0x35,        //   Usage (Rz)
		0x15, 0x00,        //   Logical Minimum (0)
		0x26, 0xFF, 0x00,  //   Logical Maximum (255)
		0x75, 0x08,        //   Report Size (8)
		0x95, 0x04,        //   Report Count (4)
		0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
		0xC0,              // End Collection
	};

	WINUHID_DEVICE_CONFIG config =
	{
		WINUHID_EVENT_NONE,
		0x1234,
		0x5678,
		0,
		sizeof(reportDescriptor),
		reportDescriptor,
		{},
		NULL,
		NULL,
		0
	};

	PWINUHID_DEVICE device = WinUHidCreateDevice(&config);
	ASSERT_NE(device, nullptr);

	ASSERT_TRUE(WinUHidStartDevice(device, NULL, NULL));

	SDLHIDManager hm;
	SDL_hid_device* hid = hm.OpenDevice(config.VendorID, config.ProductID);
	ASSERT_NE(device, nullptr);

	UCHAR expectedData[] { 1, 2, 3, 4 };
	UCHAR actualData[10];

	//
	// We should have no input report yet
	//
	EXPECT_EQ(SDL_hid_read_timeout(hid, actualData, sizeof(actualData), 100), 0);

	//
	// Send an input report
	//
	ASSERT_TRUE(WinUHidSubmitInputReport(device, expectedData, sizeof(expectedData)));

	//
	// Now we should see the input report from our HID client (and no extra data)
	//
	EXPECT_EQ(SDL_hid_read_timeout(hid, actualData, sizeof(actualData), 100), 4);

	//
	// The report should be identical to what we sent
	//
	EXPECT_TRUE(RtlEqualMemory(actualData, expectedData, sizeof(expectedData)));

	SDL_hid_close(hid);
	WinUHidDestroyDevice(device);
}

TEST(HIDLL, NumberedInputReport) {
	static const UCHAR reportDescriptorNumbered[] =
	{
		0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
		0x09, 0x05,        // Usage (Game Pad)
		0xA1, 0x01,        // Collection (Application)
		0x85, 0x01,        //   Report ID (1)
		0x09, 0x30,        //   Usage (X)
		0x09, 0x31,        //   Usage (Y)
		0x09, 0x32,        //   Usage (Z)
		0x09, 0x35,        //   Usage (Rz)
		0x15, 0x00,        //   Logical Minimum (0)
		0x26, 0xFF, 0x00,  //   Logical Maximum (255)
		0x75, 0x08,        //   Report Size (8)
		0x95, 0x04,        //   Report Count (4)
		0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
		0xC0,              // End Collection
	};

	WINUHID_DEVICE_CONFIG config =
	{
		WINUHID_EVENT_NONE,
		0x1234,
		0x5678,
		0,
		sizeof(reportDescriptorNumbered),
		reportDescriptorNumbered,
		{},
		NULL,
		NULL,
		0
	};

	PWINUHID_DEVICE device = WinUHidCreateDevice(&config);
	ASSERT_NE(device, nullptr);

	ASSERT_TRUE(WinUHidStartDevice(device, NULL, NULL));

	SDLHIDManager hm;
	SDL_hid_device* hid = hm.OpenDevice(config.VendorID, config.ProductID);
	ASSERT_NE(device, nullptr);

	UCHAR expectedData[]{ 1, 1, 2, 3, 4 };
	UCHAR actualData[10];

	//
	// We should have no input report yet
	//
	EXPECT_EQ(SDL_hid_read_timeout(hid, actualData, sizeof(actualData), 100), 0);

	//
	// Send a numbered input report
	//
	ASSERT_TRUE(WinUHidSubmitInputReport(device, expectedData, sizeof(expectedData)));

	//
	// Now we should see the input report from our HID client (and no extra data)
	//
	EXPECT_EQ(SDL_hid_read_timeout(hid, actualData, sizeof(actualData), 100), 5);

	//
	// The report should be identical to what we sent
	//
	EXPECT_TRUE(RtlEqualMemory(actualData, expectedData, sizeof(expectedData)));

	SDL_hid_close(hid);
	WinUHidDestroyDevice(device);
}

TEST(HIDLL, OutputReport) {
	static const UCHAR reportDescriptor[] =
	{
		0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
		0x09, 0x05,        // Usage (Game Pad)
		0xA1, 0x01,        // Collection (Application)
		0x09, 0x30,        //   Usage (X)
		0x09, 0x31,        //   Usage (Y)
		0x09, 0x32,        //   Usage (Z)
		0x09, 0x35,        //   Usage (Rz)
		0x15, 0x00,        //   Logical Minimum (0)
		0x26, 0xFF, 0x00,  //   Logical Maximum (255)
		0x75, 0x08,        //   Report Size (8)
		0x95, 0x04,        //   Report Count (4)
		0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
		0x09, 0x22,        //   Usage (0x22)
		0x95, 0x01,        //   Report Count (1)
		0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
		0xC0,              // End Collection
	};

	WINUHID_DEVICE_CONFIG config =
	{
		WINUHID_EVENT_WRITE_REPORT,
		0x1234,
		0x5678,
		0,
		sizeof(reportDescriptor),
		reportDescriptor,
		{},
		NULL,
		NULL,
		0
	};

	PWINUHID_DEVICE device = WinUHidCreateDevice(&config);
	ASSERT_NE(device, nullptr);

	CallbackData<bool> callbackState;
	EXPECT_TRUE(WinUHidStartDevice(device, [](PVOID CallbackContext, PWINUHID_DEVICE Device, PCWINUHID_EVENT Event) {
		EXPECT_EQ(Event->Type, WINUHID_EVENT_WRITE_REPORT);

		//
		// Devices without numbered reports will just receive the report data with no prefix
		//
		EXPECT_EQ(Event->ReportId, 0);
		EXPECT_EQ(Event->Write.DataLength, 1);
		EXPECT_EQ(Event->Write.Data[0], 1);

		WinUHidCompleteWriteEvent(Device, Event, TRUE);

		((CallbackData<bool>*)CallbackContext)->Signal(true);
	}, &callbackState));

	SDLHIDManager hm;
	SDL_hid_device* hid = hm.OpenDevice(config.VendorID, config.ProductID);
	ASSERT_NE(device, nullptr);

	//
	// Write an output report
	//
	UCHAR outputReport[]{ 0, 1 };
	EXPECT_EQ(SDL_hid_write(hid, outputReport, sizeof(outputReport)), 2);

	//
	// Verify that our callback was invoked
	//
	EXPECT_TRUE(callbackState.Wait(true));

	SDL_hid_close(hid);
	WinUHidDestroyDevice(device);
}

TEST(HIDLL, NumberedOutputReport) {
	static const UCHAR reportDescriptor[] =
	{
		0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
		0x09, 0x05,        // Usage (Game Pad)
		0xA1, 0x01,        // Collection (Application)
		0x85, 0x01,        //   Report ID (1)
		0x09, 0x30,        //   Usage (X)
		0x09, 0x31,        //   Usage (Y)
		0x09, 0x32,        //   Usage (Z)
		0x09, 0x35,        //   Usage (Rz)
		0x15, 0x00,        //   Logical Minimum (0)
		0x26, 0xFF, 0x00,  //   Logical Maximum (255)
		0x75, 0x08,        //   Report Size (8)
		0x95, 0x04,        //   Report Count (4)
		0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
		0x85, 0x02,        //   Report ID (2)
		0x09, 0x22,        //   Usage (0x22)
		0x95, 0x01,        //   Report Count (1)
		0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
		0xC0,              // End Collection
	};

	WINUHID_DEVICE_CONFIG config =
	{
		WINUHID_EVENT_WRITE_REPORT,
		0x1234,
		0x5678,
		0,
		sizeof(reportDescriptor),
		reportDescriptor,
		{},
		NULL,
		NULL,
		0
	};

	PWINUHID_DEVICE device = WinUHidCreateDevice(&config);
	ASSERT_NE(device, nullptr);

	CallbackData<bool> callbackState;
	EXPECT_TRUE(WinUHidStartDevice(device, [](PVOID CallbackContext, PWINUHID_DEVICE Device, PCWINUHID_EVENT Event) {
		EXPECT_EQ(Event->Type, WINUHID_EVENT_WRITE_REPORT);

		//
		// Devices with numbered reports will receive the report ID followed by the report data
		//
		EXPECT_EQ(Event->ReportId, 2);
		EXPECT_EQ(Event->Write.DataLength, 2);
		EXPECT_EQ(Event->Write.Data[0], 2);
		EXPECT_EQ(Event->Write.Data[1], 1);

		WinUHidCompleteWriteEvent(Device, Event, TRUE);

		((CallbackData<bool>*)CallbackContext)->Signal(true);
		}, &callbackState));

	SDLHIDManager hm;
	SDL_hid_device* hid = hm.OpenDevice(config.VendorID, config.ProductID);
	ASSERT_NE(device, nullptr);

	//
	// Write an output report
	//
	UCHAR outputReport[]{ 2, 1 };
	EXPECT_EQ(SDL_hid_write(hid, outputReport, sizeof(outputReport)), 2);

	//
	// Verify that our callback was invoked
	//
	EXPECT_TRUE(callbackState.Wait(true));

	SDL_hid_close(hid);
	WinUHidDestroyDevice(device);
}

TEST(HIDLL, SetFeatureReport) {
	static const UCHAR reportDescriptor[] =
	{
		0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
		0x09, 0x05,        // Usage (Game Pad)
		0xA1, 0x01,        // Collection (Application)
		0x09, 0x30,        //   Usage (X)
		0x09, 0x31,        //   Usage (Y)
		0x09, 0x32,        //   Usage (Z)
		0x09, 0x35,        //   Usage (Rz)
		0x15, 0x00,        //   Logical Minimum (0)
		0x26, 0xFF, 0x00,  //   Logical Maximum (255)
		0x75, 0x08,        //   Report Size (8)
		0x95, 0x04,        //   Report Count (4)
		0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
		0x09, 0x23,        //   Usage (0x23)
		0x95, 0x01,        //   Report Count (1)
		0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
		0xC0,              // End Collection
	};

	WINUHID_DEVICE_CONFIG config =
	{
		WINUHID_EVENT_SET_FEATURE,
		0x1234,
		0x5678,
		0,
		sizeof(reportDescriptor),
		reportDescriptor,
		{},
		NULL,
		NULL,
		0
	};

	PWINUHID_DEVICE device = WinUHidCreateDevice(&config);
	ASSERT_NE(device, nullptr);

	CallbackData<bool> callbackState;
	EXPECT_TRUE(WinUHidStartDevice(device, [](PVOID CallbackContext, PWINUHID_DEVICE Device, PCWINUHID_EVENT Event) {
		EXPECT_EQ(Event->Type, WINUHID_EVENT_SET_FEATURE);

		//
		// Devices without numbered reports will just receive the report data with no prefix
		//
		EXPECT_EQ(Event->ReportId, 0);
		EXPECT_EQ(Event->Write.DataLength, 1);
		EXPECT_EQ(Event->Write.Data[0], 1);

		WinUHidCompleteWriteEvent(Device, Event, TRUE);

		((CallbackData<bool>*)CallbackContext)->Signal(true);
		}, &callbackState));

	SDLHIDManager hm;
	SDL_hid_device* hid = hm.OpenDevice(config.VendorID, config.ProductID);
	ASSERT_NE(device, nullptr);

	//
	// Send a feature report
	//
	UCHAR featureReport[]{ 0, 1 };
	EXPECT_EQ(SDL_hid_send_feature_report(hid, featureReport, sizeof(featureReport)), 2);

	//
	// Verify that our callback was invoked
	//
	EXPECT_TRUE(callbackState.Wait(true));

	SDL_hid_close(hid);
	WinUHidDestroyDevice(device);
}

TEST(HIDLL, SetNumberedFeatureReport) {
	static const UCHAR reportDescriptor[] =
	{
		0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
		0x09, 0x05,        // Usage (Game Pad)
		0xA1, 0x01,        // Collection (Application)
		0x85, 0x01,        //   Report ID (1)
		0x09, 0x30,        //   Usage (X)
		0x09, 0x31,        //   Usage (Y)
		0x09, 0x32,        //   Usage (Z)
		0x09, 0x35,        //   Usage (Rz)
		0x15, 0x00,        //   Logical Minimum (0)
		0x26, 0xFF, 0x00,  //   Logical Maximum (255)
		0x75, 0x08,        //   Report Size (8)
		0x95, 0x04,        //   Report Count (4)
		0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
		0x85, 0x03,        //   Report ID (3)
		0x09, 0x23,        //   Usage (0x23)
		0x95, 0x01,        //   Report Count (1)
		0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
		0xC0,              // End Collection
	};

	WINUHID_DEVICE_CONFIG config =
	{
		WINUHID_EVENT_SET_FEATURE,
		0x1234,
		0x5678,
		0,
		sizeof(reportDescriptor),
		reportDescriptor,
		{},
		NULL,
		NULL,
		0
	};

	PWINUHID_DEVICE device = WinUHidCreateDevice(&config);
	ASSERT_NE(device, nullptr);

	CallbackData<bool> callbackState;
	EXPECT_TRUE(WinUHidStartDevice(device, [](PVOID CallbackContext, PWINUHID_DEVICE Device, PCWINUHID_EVENT Event) {
		EXPECT_EQ(Event->Type, WINUHID_EVENT_SET_FEATURE);

		//
		// Devices with numbered reports will receive the report ID followed by the report data
		//
		EXPECT_EQ(Event->ReportId, 3);
		EXPECT_EQ(Event->Write.DataLength, 2);
		EXPECT_EQ(Event->Write.Data[0], 3);
		EXPECT_EQ(Event->Write.Data[1], 1);

		WinUHidCompleteWriteEvent(Device, Event, TRUE);

		((CallbackData<bool>*)CallbackContext)->Signal(true);
		}, &callbackState));

	SDLHIDManager hm;
	SDL_hid_device* hid = hm.OpenDevice(config.VendorID, config.ProductID);
	ASSERT_NE(device, nullptr);

	//
	// Send a feature report
	//
	UCHAR featureReport[]{ 3, 1 };
	EXPECT_EQ(SDL_hid_send_feature_report(hid, featureReport, sizeof(featureReport)), 2);

	//
	// Verify that our callback was invoked
	//
	EXPECT_TRUE(callbackState.Wait(true));

	SDL_hid_close(hid);
	WinUHidDestroyDevice(device);
}

TEST(HIDLL, GetFeatureReport) {
	static const UCHAR reportDescriptor[] =
	{
		0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
		0x09, 0x05,        // Usage (Game Pad)
		0xA1, 0x01,        // Collection (Application)
		0x09, 0x30,        //   Usage (X)
		0x09, 0x31,        //   Usage (Y)
		0x09, 0x32,        //   Usage (Z)
		0x09, 0x35,        //   Usage (Rz)
		0x15, 0x00,        //   Logical Minimum (0)
		0x26, 0xFF, 0x00,  //   Logical Maximum (255)
		0x75, 0x08,        //   Report Size (8)
		0x95, 0x04,        //   Report Count (4)
		0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
		0x09, 0x23,        //   Usage (0x23)
		0x95, 0x01,        //   Report Count (1)
		0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
		0xC0,              // End Collection
	};

	WINUHID_DEVICE_CONFIG config =
	{
		WINUHID_EVENT_GET_FEATURE,
		0x1234,
		0x5678,
		0,
		sizeof(reportDescriptor),
		reportDescriptor,
		{},
		NULL,
		NULL,
		0
	};

	PWINUHID_DEVICE device = WinUHidCreateDevice(&config);
	ASSERT_NE(device, nullptr);

	EXPECT_TRUE(WinUHidStartDevice(device, [](PVOID CallbackContext, PWINUHID_DEVICE Device, PCWINUHID_EVENT Event) {
		EXPECT_EQ(Event->Type, WINUHID_EVENT_GET_FEATURE);
		EXPECT_EQ(Event->ReportId, 0);

		UCHAR data[]{ 5 };
		WinUHidCompleteReadEvent(Device, Event, data, sizeof(data));
	}, NULL));

	SDLHIDManager hm;
	SDL_hid_device* hid = hm.OpenDevice(config.VendorID, config.ProductID);
	ASSERT_NE(device, nullptr);

	//
	// Get a feature report
	//
	UCHAR featureReport[]{ 0, 0 };
	EXPECT_EQ(SDL_hid_get_feature_report(hid, featureReport, sizeof(featureReport)), 2);

	//
	// Devices without numbered reports will return zero in the report ID value
	//
	EXPECT_EQ(featureReport[0], 0);
	EXPECT_EQ(featureReport[1], 5);

	SDL_hid_close(hid);
	WinUHidDestroyDevice(device);
}

TEST(HIDLL, GetNumberedFeatureReport) {
	static const UCHAR reportDescriptor[] =
	{
		0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
		0x09, 0x05,        // Usage (Game Pad)
		0xA1, 0x01,        // Collection (Application)
		0x85, 0x01,        //   Report ID (1)
		0x09, 0x30,        //   Usage (X)
		0x09, 0x31,        //   Usage (Y)
		0x09, 0x32,        //   Usage (Z)
		0x09, 0x35,        //   Usage (Rz)
		0x15, 0x00,        //   Logical Minimum (0)
		0x26, 0xFF, 0x00,  //   Logical Maximum (255)
		0x75, 0x08,        //   Report Size (8)
		0x95, 0x04,        //   Report Count (4)
		0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
		0x85, 0x03,        //   Report ID (3)
		0x09, 0x23,        //   Usage (0x23)
		0x95, 0x01,        //   Report Count (1)
		0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
		0xC0,              // End Collection
	};

	WINUHID_DEVICE_CONFIG config =
	{
		WINUHID_EVENT_GET_FEATURE,
		0x1234,
		0x5678,
		0,
		sizeof(reportDescriptor),
		reportDescriptor,
		{},
		NULL,
		NULL,
		0
	};

	PWINUHID_DEVICE device = WinUHidCreateDevice(&config);
	ASSERT_NE(device, nullptr);

	EXPECT_TRUE(WinUHidStartDevice(device, [](PVOID CallbackContext, PWINUHID_DEVICE Device, PCWINUHID_EVENT Event) {
		EXPECT_EQ(Event->Type, WINUHID_EVENT_GET_FEATURE);
		EXPECT_EQ(Event->ReportId, 3);

		UCHAR data[]{ 3, 5 };
		WinUHidCompleteReadEvent(Device, Event, data, sizeof(data));
	}, NULL));

	SDLHIDManager hm;
	SDL_hid_device* hid = hm.OpenDevice(config.VendorID, config.ProductID);
	ASSERT_NE(device, nullptr);

	//
	// Get a feature report
	//
	UCHAR featureReport[]{ 3, 0 };
	EXPECT_EQ(SDL_hid_get_feature_report(hid, featureReport, sizeof(featureReport)), 2);

	EXPECT_EQ(featureReport[0], 3);
	EXPECT_EQ(featureReport[1], 5);

	SDL_hid_close(hid);
	WinUHidDestroyDevice(device);
}

TEST(HIDLL, PollWakeupOnEvent) {
	static const UCHAR reportDescriptor[] =
	{
		0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
		0x09, 0x05,        // Usage (Game Pad)
		0xA1, 0x01,        // Collection (Application)
		0x09, 0x30,        //   Usage (X)
		0x09, 0x31,        //   Usage (Y)
		0x09, 0x32,        //   Usage (Z)
		0x09, 0x35,        //   Usage (Rz)
		0x15, 0x00,        //   Logical Minimum (0)
		0x26, 0xFF, 0x00,  //   Logical Maximum (255)
		0x75, 0x08,        //   Report Size (8)
		0x95, 0x04,        //   Report Count (4)
		0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
		0x09, 0x22,        //   Usage (0x22)
		0x95, 0x01,        //   Report Count (1)
		0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
		0xC0,              // End Collection
	};

	WINUHID_DEVICE_CONFIG config =
	{
		WINUHID_EVENT_WRITE_REPORT,
		0x1234,
		0x5678,
		0,
		sizeof(reportDescriptor),
		reportDescriptor,
		{},
		NULL,
		NULL,
		0
	};

	PWINUHID_DEVICE device = WinUHidCreateDevice(&config);
	ASSERT_NE(device, nullptr);

	EXPECT_TRUE(WinUHidStartDevice(device, NULL, NULL));

	SDLHIDManager hm;
	SDL_hid_device* hid = hm.OpenDevice(config.VendorID, config.ProductID);
	ASSERT_NE(device, nullptr);

	//
	// We should have no events ready to poll in the beginning
	//
	ASSERT_EQ(WinUHidPollEvent(device, 0), nullptr);
	ASSERT_EQ(GetLastError(), ERROR_TIMEOUT);

	//
	// Start our poll thread to service the output report event
	//
	std::thread pollThread([device] {
		PCWINUHID_EVENT event = WinUHidPollEvent(device, INFINITE);
		ASSERT_NE(event, nullptr);
		EXPECT_EQ(event->Type, WINUHID_EVENT_WRITE_REPORT);
		EXPECT_EQ(event->ReportId, 0);
		EXPECT_EQ(event->Write.DataLength, 1);
		EXPECT_EQ(event->Write.Data[0], 1);

		WinUHidCompleteWriteEvent(device, event, TRUE);
		});

	//
	// Sleep a bit to allow our poll thread to make it into the polling function
	//
	Sleep(1000);

	//
	// Write an output report
	//
	UCHAR outputReport[]{ 0, 1 };
	EXPECT_EQ(SDL_hid_write(hid, outputReport, sizeof(outputReport)), 2);

	//
	// Join the thread which will only work if we got an event
	//
	pollThread.join();

	SDL_hid_close(hid);
	WinUHidDestroyDevice(device);
}

TEST(HIDLL, PollWakeupOnStop) {
	static const UCHAR reportDescriptor[] =
	{
		0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
		0x09, 0x05,        // Usage (Game Pad)
		0xA1, 0x01,        // Collection (Application)
		0x09, 0x30,        //   Usage (X)
		0x09, 0x31,        //   Usage (Y)
		0x09, 0x32,        //   Usage (Z)
		0x09, 0x35,        //   Usage (Rz)
		0x15, 0x00,        //   Logical Minimum (0)
		0x26, 0xFF, 0x00,  //   Logical Maximum (255)
		0x75, 0x08,        //   Report Size (8)
		0x95, 0x04,        //   Report Count (4)
		0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
		0x09, 0x22,        //   Usage (0x22)
		0x95, 0x01,        //   Report Count (1)
		0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
		0xC0,              // End Collection
	};

	WINUHID_DEVICE_CONFIG config =
	{
		WINUHID_EVENT_NONE,
		0x1234,
		0x5678,
		0,
		sizeof(reportDescriptor),
		reportDescriptor,
		{},
		NULL,
		NULL,
		0
	};

	PWINUHID_DEVICE device = WinUHidCreateDevice(&config);
	ASSERT_NE(device, nullptr);

	EXPECT_TRUE(WinUHidStartDevice(device, NULL, NULL));

	SDLHIDManager hm;
	SDL_hid_device* hid = hm.OpenDevice(config.VendorID, config.ProductID);
	ASSERT_NE(device, nullptr);

	//
	// Start our poll thread
	//
	std::thread pollThread([device] {
		//
		// Our poll should have been cancelled when the device was stopped
		//
		ASSERT_EQ(WinUHidPollEvent(device, INFINITE), nullptr);
		ASSERT_EQ(GetLastError(), ERROR_OPERATION_ABORTED);
	});

	//
	// Sleep a bit to allow our poll thread to make it into the polling function
	//
	Sleep(1000);

	//
	// Stop the device to interrupt pending polls
	//
	WinUHidStopDevice(device);

	//
	// Join the thread which will only work if we got interrupted
	//
	pollThread.join();

	SDL_hid_close(hid);
	WinUHidDestroyDevice(device);
}
