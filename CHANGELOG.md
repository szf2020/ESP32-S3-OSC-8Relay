# Changelog

Toutes les évolutions notables du projet sont documentées ici.

---

## [v1.2.6] — Avril 2026

### Améliorations UI
- Changelog de l'onglet Système synchronisé avec GitHub Releases (fetch navigateur)
- Fallback automatique sur le CHANGELOG.md embarqué dans le firmware si hors ligne
- Lien direct vers la page GitHub Releases depuis l'UI

### Build
- Script pre-build `scripts/embed_changelog.py` : génère `include/changelog_data.h` depuis `CHANGELOG.md` à chaque compilation
- Endpoint `GET /api/changelog` : sert le CHANGELOG embarqué en PROGMEM

---

## [v1.2.5] — Avril 2026

### Améliorations UI
- Sélecteur de langue remplacé par des boutons drapeaux emoji (🇫🇷 🇬🇧 🇪🇸 🇩🇪 🇨🇳) en haut à droite
- Drapeau actif mis en évidence par un contour vert
- Portail captif : redirections dynamiques basées sur l'IP AP configurée (plus codée en dur 192.168.4.1)

### Robustesse API Web
- Validation stricte du JSON sur tous les endpoints POST (`/api/relays/*`, `/api/config/*`)
- Retour HTTP 400 explicite si payload invalide ou champ manquant

---

## [v1.2.4] — Avril 2026

### Nouvelles fonctionnalités
- Interface web multilingue : sélecteur de langue discret en haut à droite (FR, EN, ES, DE, ZH)
- Endpoint combiné `/api/live` : une seule requête HTTP par cycle (relais + système + OSC log)
- Polling chaîné côté JS (~30 ms de pause entre cycles) avec garde `liveBusy` contre l empilement

### Améliorations UI
- Log OSC : auto-scroll activable/désactivable, bouton **Clear** visible (rouge)
- Redimensionnement manuel du panneau log par glisser-déposer
- Historique log porté à 200 lignes avec préservation de la position de scroll
- Indicateur santé RAM dans l onglet Système - alerte uniquement si critique

### Performances OSC (tests Avril 2026)
| Mesure | Résultat |
|---|---|
| Débit max | **~49 559 msg/s** (1 000 msg en 20,2 ms) |
| Latence UDP send | ~114 us avg (32-233 us) |
| Toggle relais 1 (50 cycles) | ~362 us avg, 18,1 ms total |
| Burst 8 relais | ~0,83 ms (~101-107 us/msg) |
| `/relay/all` (10 cycles) | ~241 us avg |
| Ping W5500 | 0,29-0,73 ms (avg 0,46 ms) |
| Latence estimée OSC vers relais | **~0,5-1,2 ms** |

---

## [v1.2.3] — Mars 2026

### Améliorations
- Rafraîchissement log OSC x4 : 1 000 ms vers 250 ms pour suivi temps réel
- Rafraîchissement état système : 3 000 ms vers 2 000 ms
- Rafraîchissement état relais : 2 000 ms vers 1 000 ms
- UI responsive mobile : adaptation automatique smartphone/tablette (media queries)
- Grilles 4 vers 2 colonnes sur mobile, boutons pleine largeur, inputs tactiles agrandis

---

## [v1.2.2] — Mars 2026

### Nouvelles fonctionnalités
- LED bleue sur réception OSC : flash bleu 50 ms à chaque message reçu, retour automatique au vert

---

## [v1.2.1] — Mars 2026

### Corrections and améliorations
- Commande OSC `/reboot` : redémarrage à distance
- Fix LED RGB : correction de l ordre des couleurs (NEO_GRB vers NEO_RGB) pour la carte Waveshare
- Fix LED rouge permanente : suppression du try/catch parasite dans la boucle status
- Suite de tests OSC complète : 52 tests validés (relais i/f/T/F, `/relay/all`, `/ap`, `/reboot`)

---

## [v1.2.0] — Mars 2026

### Optimisations OSC
- Boucle OSC prioritaire sans throttle
- Drain complet des paquets UDP par cycle (multi-packet par pass)
- Sauvegarde NVS différée (500 ms) pour ne pas bloquer le relais
- Web/DNS throttlé à 5 ms pour libérer le CPU au profit de l OSC
- Remplacement de `delay(1)` par `yield()` dans la boucle principale
- Logs PCA9554 non-bloquants (macros `LOG_RELAY` / `LOG_ERROR`)

### Nouvelles fonctionnalités
- Portail captif DNS pour redirection automatique après connexion WiFi

### Performances OSC (tests Mars 2026)
| Mesure | Résultat |
|---|---|
| Débit | ~70 000 msg/s |
| Latence OSC vers relais | ~0,5-1,2 ms |

---

## [v1.1.0] — Mars 2026

### Nouvelles fonctionnalités
- Log unifié temps réel : messages système (vert) + OSC entrants (bleu) fusionnés dans le header
- Affichage des messages OSC avec adresse, type tag et valeur
- QR code WiFi (bibliothèque Kazuhiko Arase) dans l onglet Réseau
- Température CPU dans l onglet Système
- Timeout AP configurable (0 = toujours actif)
- Panneau Système : uptime, RAM, état ETH, relais actifs
- Boutons **ALL ON** / **ALL OFF** (web + OSC `/relay/all`)
- Commande OSC `/ap` (activation/désactivation du point d accès)
- Extinction automatique du WiFi AP après timeout sans clients
- Endpoints `/api/system/status` et `/api/osc/log`

---

## [v1.0.0] — Janvier 2026

### Version initiale
- Interface Web complète de contrôle des 8 relais
- Contrôle des relais via messages OSC UDP
- Configuration Ethernet (DHCP / IP statique)
- Support WiFi AP optionnel (activé via DI8)
