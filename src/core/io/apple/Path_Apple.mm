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
			return UNEXPECTED("Не удалось получить каталог Application Support.");

		NSString* path = paths.firstObject;
		if (!path)
			return UNEXPECTED("Не удалось получить каталог Application Support.");

		return std::filesystem::path(std::string(path.UTF8String));
	}
}