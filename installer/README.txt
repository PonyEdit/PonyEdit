WINDOWS
-------

The Windows installer requires Inno Setup(http://www.jrsoftware.org/isinfo.php). ISTool (http://www.istool.org/) is optional for a nice GUI.

To create a new build:
- Open windows.iss
- In the [Files] section, if needed, perform a search/replace on the Source directory, so it grabs files from the appropriate location. You may need to change E:\Projects\RemotED\trunk and C:\Qt\2010.05 .
- In the [Setup] section, check that OutputDir, VersionInfoVersion, AppVersion and OutputBaseFilename are correct.
- Click the Compile button.
- Click the Run button to test the installer.


MAC OS X
--------

The Windows installer requires the `macdeployqt` tool, which should be included in the Qt SDK.

To create a new distributable dmg file, build the Release version of PonyEdit, then run the following commands in the build directory:

shell> macdeployqt PonyEdit.app
shell> mkdir PonyEdit
shell> ditto --rsrc --arch i386 PonyEdit.app PonyEdit/PonyEdit.app
shell> cp ../trunk/installer/OSX/Applications PonyEdit
shell> cp ../trunk/installer/OSX/Info.plist PonyEdit/PonyEdit.app/Contents

Make sure all PonyEdit dmg files are unmounted before running this command:

shell> ../trunk/installer/osx/create-dmg --volname PonyEdit --background ../trunk/installer/osx/background.png --window-size 400 300 --icon "Applications" 300 200 --icon "PonyEdit" 50 200 --icon-size 64 PonyEdit.dmg PonyEdit

