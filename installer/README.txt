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

To create a new build, run the following command:

shell> macdeployqt PonyEdit.app -dmg