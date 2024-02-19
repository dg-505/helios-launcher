#!/bin/bash

set -e

if [ -t 0 ]; then
    # clear output
    echo -n "Clearing output..."
    rm -f ../../deploy/*.{deb,AppImage,tar.gz}
    rm -rf ../../deploy/{debian,AppImage,helios-launcher-*-lin64-portable}
    echo "done"

    # get version number from InnoSetup script
    version=$(grep "#define Version" ./win/packInstaller.iss)
    version=$(echo "${version}" | sed 's/^.\{17\}\(.*\)..$/\1/')

    echo "Deploying HELIOS++ launcher version ${version} as .deb, .AppImage and .tar.gz"

    # choose executable binary
    echo -n "Executable binary: "
    EXEC_BINARY=`zenity --file-selection --title="Select executable binary" --filename=../../build/focal/clion/release/bin/helios-launcher`
    echo "\"${EXEC_BINARY}\""

    # coose Qt directory
    echo -n  "Qt directory: "
    QT_DIR=`zenity --file-selection --directory --title="Select Qt directory" --filename=/home/${USER}/Qt/5.15.16/gcc_64/`
    echo "\"${QT_DIR}\""

    # Define root directories for each file type
    DebianRoot=../../deploy/debian/helios-launcher
    AppImageRoot=../../deploy/AppImage
    ArchiveRoot=../../deploy/helios-launcher-v0.1.0-lin64-portable

    # executable binary
    echo -n "Copy helios-launcher executable..."
    mkdir -p ${DebianRoot}/opt/helios-launcher/bin/ ${AppImageRoot}/usr/bin/ ${ArchiveRoot}/bin/
    cp ${EXEC_BINARY} ${DebianRoot}/opt/helios-launcher/bin/
    cp ${EXEC_BINARY} ${AppImageRoot}/usr/bin/
    cp ${EXEC_BINARY} ${ArchiveRoot}/bin/
    echo "done"

    # icons (not for archive)
    echo -n "Generate icons..."
    sizes=(16 22 24 32 40 48 64 72 96 128 256 512 1024)
    for size in "${sizes[@]}";
    do
        mkdir -p ${DebianRoot}/usr/local/share/icons/hicolor/${size}x${size}/apps/
        convert ./lin/heliospp.svg -resize ${size}x${size} ${DebianRoot}/usr/local/share/icons/hicolor/${size}x${size}/apps/helios-launcher.png
        mkdir -p ${AppImageRoot}/usr/share/icons/hicolor/${size}x${size}/apps/
        convert ./lin/heliospp.svg -resize ${size}x${size} ${AppImageRoot}/usr/share/icons/hicolor/${size}x${size}/apps/helios-launcher.png
    done
    mkdir -p ${DebianRoot}/usr/local/share/icons/hicolor/scalable/apps/ ${AppImageRoot}/usr/share/icons/hicolor/scalable/apps/
    cp ./lin/heliospp.svg ${DebianRoot}/usr/local/share/icons/hicolor/scalable/apps/helios-launcher.svg
    cp ./lin/heliospp.svg ${AppImageRoot}/usr/share/icons/hicolor/scalable/apps/helios-launcher.svg
    echo "done"

    # libraries (not for AppImage)
    echo -n "Collect required libraries..."
    mkdir -p ${DebianRoot}/opt/helios-launcher/lib/ ${AppImageRoot}/usr/lib/ ${ArchiveRoot}/lib/
    # Qt libraries
    libs=("libicudata.so.56" "libicui18n.so.56" "libicuuc.so.56" "libQt5Core.so.5" "libQt5DBus.so.5" "libQt5Gui.so.5" "libQt5Widgets.so.5" "libQt5XcbQpa.so.5")
    for lib in "${libs[@]}"; do
        cp ${QT_DIR}/lib/${lib} ${DebianRoot}/opt/helios-launcher/lib/
        cp ${QT_DIR}/lib/${lib} ${ArchiveRoot}/lib/
    done
    # libxcb-xinerama.so.0
    cp /usr/lib/x86_64-linux-gnu/libxcb-xinerama.so.0 ${DebianRoot}/opt/helios-launcher/lib/
    cp /usr/lib/x86_64-linux-gnu/libxcb-xinerama.so.0 ${ArchiveRoot}/lib/
    # platform plugin
    mkdir -p ${DebianRoot}/opt/helios-launcher/plugins/ ${ArchiveRoot}/plugins/
    cp -r ${QT_DIR}/plugins/platforms/ ${DebianRoot}/opt/helios-launcher/plugins/
    cp -r ${QT_DIR}/plugins/platforms/ ${ArchiveRoot}/plugins/
    echo "done"

    # Auxiliary files

    echo -n "Generate aux files..."

    # readme and license
    cp ../README.md ${DebianRoot}/opt/helios-launcher/
    cp ../README.md ${ArchiveRoot}
    cp ../LICENSE ${DebianRoot}/opt/helios-launcher/
    cp ../LICENSE ${ArchiveRoot}

    # DEBIAN maintainer scripts
    mkdir -p ${DebianRoot}/DEBIAN/
    control=$(cat <<EOF
Package: helios-launcher
Version: ${version}
Architecture: amd64
Maintainer: dg-505 <dg-505@github.com>
Homepage: https://github.com/dg-505/helios-launcher
Description: Graphical launcher for the Heidelberg LiDAR Operations Simulator (HELIOS++)
EOF
)
    echo "${control}" > ${DebianRoot}/DEBIAN/control
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
    echo "${postrm}" > ${DebianRoot}/DEBIAN/postrm
    chmod -R 775 ${DebianRoot}/DEBIAN/*

    # startup script in $PATH
    mkdir -p ${DebianRoot}/usr/local/bin
    scriptPath=$(cat <<EOF
#!/bin/sh

LD_LIBRARY_PATH=/opt/helios-launcher/lib/ /opt/helios-launcher/bin/helios-launcher
EOF
)
    echo "${scriptPath}" > ${DebianRoot}/usr/local/bin/helios-launcher
    chmod 744 ${DebianRoot}/usr/local/bin/helios-launcher

    # .desktop file for debian package
    mkdir -p ${DebianRoot}/usr/local/share/applications
    desktopDebian=$(cat <<EOF
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
    echo "${desktopDebian}" > ${DebianRoot}/usr/local/share/applications/helios-launcher.desktop

    # .desktop file for AppImage
    mkdir -p ${AppImageRoot}/usr/share/applications
    desktopAppImage=$(cat <<EOF
[Desktop Entry]
Type        = Application
Name        = HELIOS++ launcher
GenericName = Graphical launcher for the Heidelberg LiDAR Operations Simulator (HELIOS++)
Terminal    = true
Exec        = helios-launcher
Icon        = helios-launcher
Comment     = HELIOS++ launcher provides a convenient way to run HELIOS++ from a graphical user interface
Categories  = Utility;
EOF
)
    echo "${desktopAppImage}" > ${AppImageRoot}/usr/share/applications/helios-launcher.desktop

    # startup script in archive root
    scriptArchive=$(cat <<EOF
#!/bin/bash

LD_LIBRARY_PATH=./lib ./bin/helios-launcher

EOF
)
    echo "${scriptArchive}" > ${ArchiveRoot}/helios-launcher.sh
    chmod 744 ${ArchiveRoot}/helios-launcher.sh

    echo "done"

    echo "Packing deploy files..."
    dpkg-deb --build ${DebianRoot}
    mv ${DebianRoot}*.deb ../../deploy/helios-launcher-v${version}-lin64.deb
    ~/Downloads/linuxdeployqt-continuous-x86_64.AppImage ${AppImageRoot}/usr/share/applications/helios-launcher.desktop -qmake=${QT_DIR}/bin/qmake -appimage -no-translations
    mv ./*.AppImage ../../deploy/helios-launcher-v${version}-lin64.AppImage
    tar -czvf ${ArchiveRoot}.tar.gz -C ../../deploy/ helios-launcher-v${version}-lin64-portable
else
       gnome-terminal -e "bash -c './deploy-lin64.sh; read -p \"Press Enter to exit...\"'"
fi

exit
