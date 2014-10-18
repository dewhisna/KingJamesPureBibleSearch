#ifndef VERSION_ISS
#define VERSION_ISS
#ifndef EXTRAVERSION
#define EXTRAVERSION ""
#endif

#ifdef SpecialBuild
  #define SpecialBuildString SpecialBuild
#else
  #define SpecialBuildString ""
#endif
#define INSTALLER_VERSION

#define GetCleanFileVersion(str FileName) \
	ParseVersion(FileName,Local[1],Local[2],Local[3],Local[4]), \
	Str(Local[1]) + "." + Str(Local[2]) \
	+ (Local[3] || Local[4] ? "." + Str(Local[3]) : "") \
	+ (Local[4] ? "." + Str(Local[4]) : "" )

#define GetCleanFileVersionVB(str FileName) \
	ParseVersion(FileName,Local[1],Local[2],Local[3],Local[4]), \
	Str(Local[1]) + "." + Str(Local[2]) \
	+ (Local[4] ? "." + Str(Local[4]) : "" )

#define VersionInstaller(str AppName) \
  SpecialBuildString = SpecialBuildString ? SpecialBuildString : GetStringFileInfo(AppName,"SpecialBuild"), \
  VersionInstallerTo(GetCleanFileVersion(AppName))
#define VersionInstallerVB(str AppName) VersionInstallerTo(GetCleanFileVersionVB(AppName))
#define VersionInstallerProduct(str AppName) VersionInstallerTo(GetStringFileInfo(AppName,PRODUCT_VERSION))

#define VersionInstallerTo(str version) \
	INSTALLER_VERSION = version, \
  Local[1] = SpecialBuildString ? " (" + SpecialBuildString + ")" : "", \
	SetSetupSetting("AppVerName", SetupSetting("AppName") + " " + version + (EXTRAVERSION ? "-" : "") + EXTRAVERSION + Local[1]), \
	SetSetupSetting("AppVersion", version + (EXTRAVERSION ? "-" : "") + EXTRAVERSION + Local[1]), \
	SetSetupSetting("OutputBaseFilename", StringChange(SetupSetting("AppName"), " ", "") + "-" + version + (EXTRAVERSION ? "-" : "") + EXTRAVERSION), \
	SetSetupSetting("VersionInfoVersion", version);
#endif
