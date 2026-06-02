#import <Foundation/Foundation.h>

#include <string>
#include <expected>
#include <filesystem>

#include "../Path.h"

std::expected<std::filesystem::path, std::string> zzz::io::Path::GetAppleUserDataDirectory()
{
	@autoreleasepool
	{
		NSArray* paths =
			NSSearchPathForDirectoriesInDomains(
				NSApplicationSupportDirectory,
				NSUserDomainMask,
				YES);

		if (paths.count == 0)
			UNEXPECTED("Failed to get Application Support directory.");

		NSString* path = paths.firstObject;
		if (!path)
			UNEXPECTED("Failed to get Application Support directory.");

		return std::filesystem::path(std::string(path.UTF8String));
	}
}