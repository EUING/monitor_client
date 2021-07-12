#include "local_remove_event.h"

#include <Windows.h>
#include <shobjidl_core.h>

#include <iostream>
#include <optional>
#include <memory>

#include "item_request.h"

namespace monitor_client {
	bool LocalRemoveEvent::operator()(ItemRequest& item_request) const {	
		auto perform = [](const std::wstring& relative_path) {
			auto release = [](IUnknown* unknown) {
				if (unknown) {
					unknown->Release();
				}
			};

			wchar_t buffer[MAX_PATH] = { 0, };
			if (0 == GetFullPathName(relative_path.c_str(), sizeof(buffer) / sizeof(buffer[0]), buffer, NULL)) {
				std::wcerr << L"LocalRemoveEvent::operator(): GetFullPathName Fail: " << relative_path << std::endl;
				return false;
			}

			std::wstring full_path(buffer);
			std::unique_ptr<IFileOperation, decltype(release)> file_operation_ptr(nullptr, release);
			std::unique_ptr<IShellItem, decltype(release)> item_ptr(nullptr, release);

			HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
			if (!SUCCEEDED(hr)) {
				std::wcerr << L"LocalRemoveEvent::operator(): CoInitializeEx Fail: " << full_path << std::endl;
				return false;
			}

			IFileOperation* file_operation = nullptr;
			hr = CoCreateInstance(CLSID_FileOperation, NULL, CLSCTX_ALL, IID_PPV_ARGS(&file_operation));
			if (!SUCCEEDED(hr)) {
				std::wcerr << L"LocalRemoveEvent::operator(): CoCreateInstance Fail: " << full_path << std::endl;
				return false;
			}
			file_operation_ptr.reset(file_operation);

			DWORD flags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_ALLOWUNDO;
			hr = file_operation_ptr->SetOperationFlags(flags);
			if (!SUCCEEDED(hr)) {
				std::wcerr << L"LocalRemoveEvent::operator(): SetOperationFlags Fail: " << full_path << std::endl;
				return false;
			}

			IShellItem* item = nullptr;
			hr = SHCreateItemFromParsingName(full_path.c_str(), NULL, IID_PPV_ARGS(&item));
			if (!SUCCEEDED(hr)) {
				std::wcerr << L"LocalRemoveEvent::operator(): SHCreateItemFromParsingName Fail: " << full_path << std::endl;
				return false;
			}
			item_ptr.reset(item);

			hr = file_operation_ptr->DeleteItem(item_ptr.get(), NULL);
			if (!SUCCEEDED(hr)) {
				std::wcerr << L"LocalRemoveEvent::operator(): DeleteItem Fail: " << full_path << std::endl;
				return false;
			}

			file_operation_ptr->PerformOperations();

			return SUCCEEDED(hr);
		};

		bool result = perform(relative_path_);
		CoUninitialize();

		if (!result) {
			std::wcerr << L"LocalRemoveEvent::operator(): PerformOperations Fail: " << relative_path_ << std::endl;
			return false;
		}

		return item_request.LocalRemoveRequest(relative_path_);
	}
}  // namespace monitor_client