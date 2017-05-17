#pragma once

namespace freeimagecpp
{
class CFreeImage;
class CFreeImageMulti;
typedef std::unique_ptr<CFreeImage> CFreeImageUniquePtr;
typedef std::shared_ptr<CFreeImage> CFreeImageSharedPtr;
typedef std::shared_ptr<const CFreeImage> CConstFreeImageSharedPtr;

class IFreeImageIO;

class CFreeImageIOOstream;

class CFreeImageIO;  // became visible in CFreeImageMulti interface
}
