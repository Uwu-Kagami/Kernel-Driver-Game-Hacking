<p align="center">
  <img src="https://raw.githubusercontent.com/Uwu-Kagami/Uwu-Kagami/refs/heads/main/gif/c88cc62241ed6cb2b0fb68a83e493cf9.gif" width="300"/>
</p>
<p align="center">
  <a href="https://raw.githubusercontent.com/Uwu-Kagmi/Anti-Debugger/main/LICENSE">
    <img src="https://img.shields.io/badge/License-MIT-red?style=flat-square">
  </a>
  <a href="https://github.com/Uwu-Kagami/Discord-Extension-Token-Grabber">
    <img src="https://img.shields.io/github/repo-size/Uwu-Kagami/Discord-Extension-Token-Grabber?style=flat-square">
  </a>
</p>



## 🛡️ CS2 Undetected Usermode Driver (External)

> Un driver **usermode** pour **Counter-Strike 2**, conçu pour lire et écrire la mémoire sans être détecté par les anti-cheats en espace utilisateur.  
> ⚠️ **Ce driver ne bypass PAS les anti-cheats fonctionnant en Ring 0** (noyau).

---

## 🚀 Fonctionnalités

- 🔍 Lecture et écriture mémoire externe
- 🔧 Interface simple avec `DeviceIoControl`
- 👻 Indétectable par les anti-cheats usermode (VAC, FACEIT Client, etc.)
- 💻 Compatible avec tous les projets externes (ESP, Aimbot, Bunnyhop…)

---

## ⚙️ Prérequis

- Windows 10 ou 11 (x64)
- Visual Studio (C++17 minimum)
- Accès administrateur pour lancer le driver

---

## 🛠️ Installation & Utilisation

### 1. Compilation

- Ouvre le projet `.sln` dans Visual Studio
- Compile en mode **Release x64**

### 2. Chargement du Driver

- Lancement via ton propre loader usermode
- Ou création d’un service avec `CreateService()` / `StartService()` (optionnel)

### 3. Communication avec le Driver

Tu peux ensuite lire ou écrire la mémoire du jeu via `DeviceIoControl` :

## ⚙️ Mapping avec kdMapper

1. Télécharge [kdMapper](https://github.com/TheCruZ/kdmapper)  
2. Compile ton driver en `.sys` (Release x64)
3. Drag & Drop ton sys dans le kdMapper.exe
4. Mappe ton driver avec :
