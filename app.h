#pragma once
#include <cz_app.h>
#include <stdbool.h>
#include <stdint.h>

// ─── Dimensions écran CardputerZero ───────────────────────────────────────────
#define CZ_W          320
#define CZ_H          170
#define STATUSBAR_H    18
#define INPUT_H        24
#define CONTENT_H     (CZ_H - STATUSBAR_H - INPUT_H)  // 128px

// ─── Palette couleurs (thème radio/terminal sombre) ───────────────────────────
#define COL_BG          0x0D1117   // fond quasi-noir
#define COL_SURFACE     0x161B22   // panneaux
#define COL_BORDER      0x30363D   // séparateurs
#define COL_GREEN       0x39D353   // signal / texte perso
#define COL_BLUE        0x58A6FF   // nœuds / accent
#define COL_YELLOW      0xE3B341   // avertissements
#define COL_GRAY        0x8B949E   // texte secondaire
#define COL_WHITE       0xF0F6FC   // texte principal
#define COL_RED         0xF85149   // erreur / hors ligne

// ─── Nombre max de messages / nœuds affichés ──────────────────────────────────
#define MAX_MESSAGES    32
#define MAX_NODES       16
#define MSG_TEXT_LEN   128
#define NODE_NAME_LEN   32

// ─── Structures de données ────────────────────────────────────────────────────

typedef struct {
    char    from[NODE_NAME_LEN];
    char    text[MSG_TEXT_LEN];
    char    time[6];   // "HH:MM\0"
    bool    is_mine;
} MeshMessage;

typedef struct {
    char     name[NODE_NAME_LEN];
    int8_t   rssi;       // dBm
    bool     has_gps;
    bool     online;
    uint32_t last_seen;  // timestamp unix
} MeshNode;

// ─── État global de l'app ─────────────────────────────────────────────────────

typedef struct {
    MeshMessage messages[MAX_MESSAGES];
    int         msg_count;

    MeshNode    nodes[MAX_NODES];
    int         node_count;

    char        my_name[NODE_NAME_LEN];
    char        channel[32];
    char        region[16];

    bool        daemon_connected;  // meshtasticd répond ?
} AppState;

extern AppState g_state;

// ─── Écrans ───────────────────────────────────────────────────────────────────

typedef enum {
    SCREEN_MESSAGES = 0,
    SCREEN_NODES,
    SCREEN_CONFIG,
    SCREEN_COUNT
} ScreenId;

// Créateurs d'écrans
lv_obj_t *screen_messages_create(lv_obj_t *parent);
lv_obj_t *screen_nodes_create(lv_obj_t *parent);
lv_obj_t *screen_config_create(lv_obj_t *parent);

// Refresh (appelé par le timer de poll)
void screen_messages_refresh(void);
void screen_nodes_refresh(void);

// Événements clavier transmis par main
void screen_messages_on_key(uint32_t key);
void screen_nodes_on_key(uint32_t key);
void screen_config_on_key(uint32_t key);

// ─── API Meshtastic (mock → vrai HTTP plus tard) ──────────────────────────────

void  mesh_api_init(void);
bool  mesh_api_poll(void);         // retourne true si données fraîches
bool  mesh_api_send(const char *text);

// ─── Helpers UI ───────────────────────────────────────────────────────────────

lv_obj_t *ui_statusbar_create(lv_obj_t *parent);
void      ui_statusbar_update(int node_count, bool connected);

static inline lv_color_t hex(uint32_t v) {
    return lv_color_make((v >> 16) & 0xFF, (v >> 8) & 0xFF, v & 0xFF);
}
