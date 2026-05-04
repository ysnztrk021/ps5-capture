# PS5 Capture 🎮

Une application Linux pour afficher et écouter le flux HDMI de votre PlayStation 5 directement dans une fenêtre, sans logiciel tiers.

---

## Aperçu

PS5 Capture est une application Qt6 + GStreamer qui permet de :
- Afficher le flux vidéo HDMI de la PS5 dans une fenêtre intégrée
- Écouter l'audio de la PS5 directement dans l'application via PipeWire
- Passer en plein écran avec un bouton ou la touche `F11`
- Détecter automatiquement la carte de capture et le périphérique audio

---

## Prérequis

- Linux avec PipeWire (Nobara, Fedora, Bazzite...)
- Une carte de capture HDMI USB compatible V4L2
- Qt6
- GStreamer 1.0

---

## Installation

```bash
git clone https://github.com/votre-utilisateur/ps5-capture.git
cd ps5-capture
chmod +x install.sh
./install.sh
```

Le script va automatiquement :
1. Installer les dépendances via `dnf`
2. Compiler l'application
3. Copier le binaire dans `~/.local/bin/`
4. Installer l'icône dans `~/.local/share/icons/`
5. Créer un raccourci dans le menu des applications

---

## Utilisation

Lancez l'application depuis le menu ou en terminal :

```bash
ps5_capture
```

- Sélectionnez votre carte de capture dans la liste **Vidéo**
- Sélectionnez votre source audio dans la liste **Audio** (sélection automatique)
- Cliquez sur **▶ Lancer** pour démarrer
- Cliquez sur **⏹ Arrêter** pour stopper
- **F11** ou **⛶ Plein écran** pour passer en plein écran
- **Échap** pour quitter le plein écran

---

## Dépendances

| Paquet | Rôle |
|--------|------|
| `qt6-qtbase-devel` | Interface graphique |
| `cmake` | Compilation |
| `gcc-c++` | Compilateur C++ |
| `pkg-config` | Détection des bibliothèques |
| `gstreamer1-devel` | Capture vidéo/audio |
| `gstreamer1-plugins-base-devel` | Plugins GStreamer de base |

---

## Structure du projet
```text
ps5-capture/
├── CMakeLists.txt         # Configuration de compilation
├── main.cpp               # Point d'entrée
├── mainwindow.h/.cpp      # Fenêtre principale
├── gstwidget.h/.cpp       # Widget vidéo GStreamer
├── resources.qrc          # Ressources Qt (icône)
├── Ps5_Logo.png           # Icône de l'application
└── install.sh             # Script d'installation
```
---

## Compatibilité

| Distribution | Statut |
|-------------|--------|
| Nobara Linux | ✅ Testé |
| Fedora | ✅ Compatible |
| Bazzite | ⚠️ Utiliser `rpm-ostree` à la place de `dnf` |

---

## Problèmes connus

- **Bazzite OS** : `mpv-libs-devel` a des conflits de dépendances, utiliser la version GStreamer
- **Wayland** : `xvimagesink` ne fonctionne pas, l'application utilise un rendu par `appsink` + `QLabel` pour contourner ce problème
- Si la carte de capture n'apparaît pas, vérifiez que vous êtes bien dans le groupe `video` :
```bash
  sudo usermod -aG video $USER
```

---

## Licence

MIT License — libre d'utilisation, de modification et de distribution.

---

## Auteur

Fait avec ❤️ pour la communauté PlayStation sur Linux.
