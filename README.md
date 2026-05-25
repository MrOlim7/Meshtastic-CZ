# Meshtastic CZ — App Meshtastic pour M5Stack CardputerZero

App native pour le CardputerZero qui affiche les messages et nœuds
Meshtastic via le daemon `meshtasticd` (Linux natif + SX1262 via SPI).

## Architecture

```
meshtastic-cz/
├── app-builder.json          ← manifest czdev (nom, runtime LVGL)
├── CMakeLists.txt            ← build czdev
├── include/
│   └── app.h                 ← types, constantes, déclarations partagées
└── src/
    ├── main.c                ← app_main(), app_event(), navigation
    ├── meshtastic_api.c      ← mock API → vrais appels HTTP meshtasticd
    ├── screen_messages.c     ← écran chat (liste + saisie clavier)
    ├── screen_nodes.c        ← écran nœuds du réseau
    └── screen_config.c       ← écran paramètres
```

## Lancer dans l'émulateur czdev

```bash
# Cloner le CardputerZero-AppBuilder avec submodules
git clone --recursive https://github.com/m5stack/CardputerZero-AppBuilder.git
cd CardputerZero-AppBuilder

# Installer les dépendances (Linux Debian/Ubuntu)
sudo apt install -y build-essential cmake pkg-config \
    libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libfreetype-dev
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh

# Vérifier l'environnement
cargo run -p czdev --release -- doctor

# Copier notre app dans le dossier apps/
cp -r /chemin/vers/meshtastic-cz apps/meshtastic_cz

# Lancer (hot-reload automatique à chaque modif)
cargo run -p czdev --release -- watch apps/meshtastic_cz
```

## Navigation clavier (CardputerZero)

| Touche       | Action                        |
|-------------|-------------------------------|
| Fn + M      | → Écran Messages              |
| Fn + N      | → Écran Nœuds                 |
| Fn + S      | → Écran Config / Paramètres   |
| Tab         | Basculer écran suivant        |
| Escape      | Retour à Messages             |
| Entrée      | Envoyer le message saisi      |
| Backspace   | Effacer un caractère          |
| ↑ / ↓       | Scroller la liste             |

## Roadmap

### Phase 1 — UI (maintenant, sans hardware)
- [x] Écran Messages avec liste scrollable et saisie clavier
- [x] Écran Nœuds avec RSSI et état GPS
- [x] Écran Config (lecture seule)
- [x] Mock API (données simulées)
- [x] Navigation clavier entre écrans
- [ ] Affichage heure réelle (appel `time()`)
- [ ] Scroll infini / pagination messages

### Phase 2 — Intégration meshtasticd (à la livraison, ~novembre)
- [ ] Remplacer mock par vrais appels HTTP `localhost:4403`
  - GET  `/api/v1/fromradio`  → messages entrants
  - POST `/api/v1/toradio`   → envoyer
  - GET  `/api/v1/nodes`     → liste nœuds
- [ ] Service systemd `meshtasticd` avec config SX1262 SPI
- [ ] Adapter les keycodes Fn aux vrais keycodes du CardputerZero

### Phase 3 — Fonctionnalités avancées
- [ ] Canaux multiples
- [ ] Carte GPS des nœuds (si HDMI branché)
- [ ] Notifications sonores (haut-parleur 1W)
- [ ] Publication sur l'AppStore CardputerZero (`czdev publish`)

## Configuration meshtasticd pour SX1262 (Cap LoRa-1262)

Fichier `/etc/meshtasticd/config.yaml` sur le CardputerZero :

```yaml
Lora:
  Module: sx1262
  CS: 8       # à vérifier selon pinout Cap LoRa sur CZ
  IRQ: 16
  Busy: 20
  Reset: 24
  DIO3_TCXO_VOLTAGE: 1.8

General:
  MaxNodes: 200

Webserver:
  Port: 4403
  RootPath: /usr/share/meshtasticd/web
```

## Palette couleurs (thème radio/terminal)

| Couleur      | Hex       | Usage                        |
|-------------|-----------|------------------------------|
| Fond        | `#0D1117` | Arrière-plan général         |
| Surface     | `#161B22` | Statusbar, header            |
| Bordure     | `#30363D` | Séparateurs                  |
| Vert        | `#39D353` | Nos messages, daemon OK      |
| Bleu        | `#58A6FF` | Nœuds en ligne, accent       |
| Jaune       | `#E3B341` | Signal faible, config        |
| Gris        | `#8B949E` | Texte secondaire             |
| Blanc       | `#F0F6FC` | Texte principal              |
| Rouge       | `#F85149` | Hors ligne, erreur           |
