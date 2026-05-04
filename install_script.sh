#!/bin/bash
set -e

echo "Installation de PS5 Capture..."

# Installation des dépendances
sudo dnf install -y qt6-qtbase-devel cmake gcc-c++ pkg-config \
    gstreamer1-devel gstreamer1-plugins-base-devel

# Compilation
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)

# Installation du binaire
mkdir -p ~/.local/bin
cp ps5_capture ~/.local/bin/
cd .. && rm -rf build

# Installation de l'icône
mkdir -p ~/.local/share/icons
cp Ps5_Logo.png ~/.local/share/icons/

# Création du fichier .desktop
mkdir -p ~/.local/share/applications
cat > ~/.local/share/applications/Playstation.desktop << EOF
[Desktop Entry]
Categories=Game;
Comment[fr_FR]=HDMI CAPTOR FOR PS5
Comment=Playstation 5
Exec=$HOME/.local/bin/ps5_capture
GenericName[fr_FR]=
GenericName=
Icon=$HOME/.local/share/icons/Ps5_Logo.png
Name[fr_FR]=Playstation 5
Name=Playstation 5
StartupNotify=true
Terminal=false
Type=Application
X-KDE-SubstituteUID=false
EOF

# Mettre à jour la base de données des applications
update-desktop-database ~/.local/share/applications 2>/dev/null || true

echo "Installation terminée !"
echo "PS5 Capture est disponible dans le menu des applications."
