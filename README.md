# helios-launcher

Graphical launcher for the [Heidelberg LiDAR Operations Simulator (HELIOS++)](https://www.geog.uni-heidelberg.de/gis/helios.html) using the [Qt framework](https://www.qt.io/)

You need a working installation of HELIOS++ from [GitHub](https://github.com/3dgeo-heidelberg/helios/releases).

Some settings like the HELIOS++ base directory, survey path or optional arguments are saved between sessions. These settings are stored as an *.ini
configuration file in the default QSettings directory:
- For Linux, this is usually `~/.config/heliospp/helios-launcher.ini`
- For Windows, this is usually `C:\Users\<username>\AppData\Roaming\heliospp\helios-launcher.ini`

## Installation (Linux)

### Debian package (most recommended)
- Open the `helios-launcher-v{VERSION}-lin64.deb` in your software center
- Hit the `Install` button and enter your password
- To uninstall, open the `helios-launcher-v{VERSION}-lin64.deb` again in your software center and click `Uninstall` (This will keep the settings file)

Alternatively:
- Open the the directory that contains `helios-launcher-v{VERSION}-lin64.deb` in the terminal
- Execute `sudo dpkg -i helios-launcher-v{VERSION}-lin64.deb` and enter your password
- To uninstall, execute `sudo dpkg -r helios-launcher` to keep the settings file or `sudo dpkg --purge helios-launcher` to remove the settings file

### AppImage
- Make the AppImage executable (e.g. via the file manager or via `chmod u+x helios-launcher-v{VERSION}-lin64.AppImage`)
- **Important notice**: AppImages need FUSE 2 to run, which isn't pre-isntalled on Ubuntu 22.04 or newer. In this case make sure to install FUSE 2 with `sudo apt install libfuse2`.

### Portable version
- Extract the `helios-launcher-v{VERSION}-lin64-portable.tar.gz` archive to an arbitrary location
- Open `/path/to/helios-launcher-v{VERSION}-lin64-portable` in the terminal
- Execute the shell script `./helios-launcher.sh`
- Alternatively: Navigate to the `bin` subdirectory and execute  `LD_LIBRARY_PATH=../lib ./helios-launcher`

## Installation (Windows)

- Install by running the setup: `helios-launcher-v{VERSION}-win64-setup.exe`
- **OR:** Extract the zip file `helios-launcher-v0.1.0-win64-portable.zip`, go to the extracted folder and to the `bin` subdirectory, and run `helios-launcher.exe`

## Usage
At the first program start you have to specify the HELIOS++ installation directory (i.e. the folder that contains the `data`, `pyhelios`, `run`, etc. folders).

- In the UI the first line specifies the path to the HELIOS++ installation directory.
- The second line conatins the path to a [`survey.xml`](https://github.com/3dgeo-heidelberg/helios/wiki/Survey).
- In the third line you have to specify if you want to use the normal HELIOS++ executable (`run/helios` on Linux or `run\helios.exe` on Windows) or the python script `run/helios.py`.
- To set all the other optional arguments you can use either the UI elements, or the Argument editor.
- Before running HELIOS++ with the selected options, you have to specify if you want to use the UI elements *OR* the argument editor as the command source. The command to be executed is displayed at the bottom of the window.
- Hit the `Run` button or use `Alt+R` to execute HELIOS++ with the selected arguments. The output is displayed on the right.
