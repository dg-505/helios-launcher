#!/bin/bash

set -e

if [ -t 0 ]; then
    # clear output
    echo -n "Clearing output..."
    rm -f ../../release/*.deb
    rm -rf ../../release/deb
    echo "done"

    # get version number from InnoSetup script
    version=$(grep "#define Version" ./win/packInstaller.iss)
    version=$(echo "${version}" | sed 's/^.\{17\}\(.*\)..$/\1/')
    echo "Packaging HELIOS++ launcher version ${version}"

    ### generate DEBIAN directory with control file and postrm script
    echo -n "Generating maintainer scripts in 'DEBIAN' directory..."
    mkdir -p ../../release/deb/helios-launcher/DEBIAN
    control=$(cat <<EOF
Package: helios-launcher
Version: $version
Architecture: amd64
Maintainer: dg-505 <dg-505@github.com>
Homepage: https://github.com/dg-505/helios-launcher
Description: Graphical launcher for the Heidelberg LiDAR Operations Simulator (HELIOS++)
EOF
)
    echo "${control}" > ../../release/deb/helios-launcher/DEBIAN/control

    postrm=$(cat <<EOF
#!/bin/bash

set -e

if [ "\$1" == "purge" ]; then
    cfgDir="/home/\${SUDO_USER}/.config/heliospp/"
    if [ -d "\${cfgDir}" ]; then
        rm -rf \${cfgDir}
    fi
fi
EOF
)
    echo "${postrm}" > ../../release/deb/helios-launcher/DEBIAN/postrm

    chmod -R 775 ../../release/deb/helios-launcher/DEBIAN/*

    echo "done"

    ### generate shell script in /usr/local/bin to call the executable binary from /opt/helios-launcher/bin/helios-launcher
    mkdir -p ../../release/deb/helios-launcher/usr/local/bin
    script=$(cat <<EOF
#!/bin/sh
LD_LIBRARY_PATH=/opt/helios-launcher/lib/ /opt/helios-launcher/bin/helios-launcher
EOF
)
    echo "${script}" > ../../release/deb/helios-launcher/usr/local/bin/helios-launcher
    chmod +x ../../release/deb/helios-launcher/usr/local/bin/helios-launcher

    ### generate "/opt/helios-launcher/bin" directory with executable binary
    mkdir -p ../../release/deb/helios-launcher/opt/helios-launcher/bin
    # choose executable binary
    EXEC_BINARY=`zenity --file-selection --title="Select executable binary" --filename=../../build/focal/clion/release/bin/helios-launcher`
    echo "Executable binary: \"${EXEC_BINARY}\""

    # coose Qt directory
    QT_DIR=`zenity --file-selection --directory --title="Select Qt directory" --filename=/home/${USER}/Qt/5.15.16/gcc_64/`
    echo "Qt directory: \"${QT_DIR}\""

    echo -n "Copying package files..."

    cp ${EXEC_BINARY} ../../release/deb/helios-launcher/opt/helios-launcher/bin/

    # readme and license
    cp ../README.md ../../release/deb/helios-launcher/opt/helios-launcher/
    cp ../LICENSE ../../release/deb/helios-launcher/opt/helios-launcher/

    # platform plugin
    mkdir -p ../../release/deb/helios-launcher/opt/helios-launcher/plugins
    cp -r ${QT_DIR}/plugins/platforms ../../release/deb/helios-launcher/opt/helios-launcher/plugins/

    # libaries required by Qt
    mkdir -p ../../release/deb/helios-launcher/opt/helios-launcher/lib
    libs=("libicudata.so.56" "libicui18n.so.56" "libicuuc.so.56" "libQt5Core.so.5" "libQt5DBus.so.5" "libQt5Gui.so.5" "libQt5Widgets.so.5" "libQt5XcbQpa.so.5")
    for lib in "${libs[@]}"; do
        cp ${QT_DIR}/lib/${lib} ../../release/deb/helios-launcher/opt/helios-launcher/lib/
    done

    # libxcb-xinerama.so.0 libxcb-xinerama.so.0.0.0
    cp /usr/lib/x86_64-linux-gnu/libxcb-xinerama.so.0 ../../release/deb/helios-launcher/opt/helios-launcher/lib/

    ### generate "/usr/local/share" directory with .desktop file and icons
    mkdir -p ../../release/deb/helios-launcher/usr/local/share/applications
    desktop=$(cat <<EOF
[Desktop Entry]
Type        = Application
Name        = HELIOS++ launcher
GenericName = Graphical launcher for the Heidelberg LiDAR Operations Simulator (HELIOS++)
Terminal    = true
Version     = ${version}
Exec        = helios-launcher
Icon        = helios-launcher
Comment     = HELIOS++ launcher provides a convenient way to run HELIOS++ from a graphical user interface
Categories  = Utility;
EOF
)
    echo "${desktop}" > ../../release/deb/helios-launcher/usr/local/share/applications/helios-launcher.desktop

    sizes=(16 22 24 32 40 48 64 72 96 128 256 512 1024)
    for size in "${sizes[@]}";
    do
        mkdir -p ../../release/deb/helios-launcher/usr/local/share/icons/hicolor/${size}x${size}/apps/
        convert ./lin/heliospp.svg -resize ${size}x${size} ../../release/deb/helios-launcher/usr/local/share/icons/hicolor/${size}x${size}/apps/helios-launcher.png
    done
    mkdir -p ../../release/deb/helios-launcher/usr/local/share/icons/hicolor/scalable/apps/
    cp ./lin/heliospp.svg ../../release/deb/helios-launcher/usr/local/share/icons/hicolor/scalable/apps/helios-launcher.svg

    echo "done"

    ### build deb package
    dpkg-deb --build ../../release/deb/helios-launcher

    mv ../../release/deb/helios-launcher*.deb ../../release/helios-launcher-v${version}-lin64.deb

else
    gnome-terminal -e "bash -c './release-lin64-deb.sh; read -p \"Press Enter to exit...\"'"
fi

exit
