#include "app.h"
#include <string.h>
#include <stdio.h>

// ─────────────────────────────────────────────────────────────────────────────
//  meshtastic_api.c
//
//  Pour l'instant : données mockées pour développer l'UI sans le hardware.
//  Plus tard : remplacer mesh_api_poll() et mesh_api_send() par des appels
//  HTTP GET/POST vers http://localhost:4403 (meshtasticd web API).
//
//  Endpoints réels meshtasticd :
//    GET  /api/v1/fromradio          → messages entrants (protobuf/JSON)
//    POST /api/v1/toradio            → envoyer un message
//    GET  /api/v1/nodes              → liste des nœuds
// ─────────────────────────────────────────────────────────────────────────────

AppState g_state = {0};

// Compteur pour simuler des messages qui arrivent progressivement
static int  _mock_tick   = 0;
static bool _initialized = false;

void mesh_api_init(void) {
    if (_initialized) return;
    _initialized = true;

    // ── Infos de notre nœud ──────────────────────────────────────────────────
    snprintf(g_state.my_name, sizeof(g_state.my_name), "MonNoeud");
    snprintf(g_state.channel, sizeof(g_state.channel), "LongFast");
    snprintf(g_state.region,  sizeof(g_state.region),  "EU_868");
    g_state.daemon_connected = true;

    // ── Nœuds initiaux ───────────────────────────────────────────────────────
    g_state.node_count = 3;

    snprintf(g_state.nodes[0].name, NODE_NAME_LEN, "Alice");
    g_state.nodes[0].rssi    = -87;
    g_state.nodes[0].has_gps = true;
    g_state.nodes[0].online  = true;

    snprintf(g_state.nodes[1].name, NODE_NAME_LEN, "Bob");
    g_state.nodes[1].rssi    = -102;
    g_state.nodes[1].has_gps = true;
    g_state.nodes[1].online  = true;

    snprintf(g_state.nodes[2].name, NODE_NAME_LEN, "Relay-1");
    g_state.nodes[2].rssi    = -118;
    g_state.nodes[2].has_gps = false;
    g_state.nodes[2].online  = false;

    // ── Messages initiaux ─────────────────────────────────────────────────────
    g_state.msg_count = 3;

    snprintf(g_state.messages[0].from, NODE_NAME_LEN, "Alice");
    snprintf(g_state.messages[0].text, MSG_TEXT_LEN,  "Salut ! Vous etes combien ?");
    snprintf(g_state.messages[0].time, 6,              "12:03");
    g_state.messages[0].is_mine = false;

    snprintf(g_state.messages[1].from, NODE_NAME_LEN, "Bob");
    snprintf(g_state.messages[1].text, MSG_TEXT_LEN,  "Je suis au sommet, signal parfait");
    snprintf(g_state.messages[1].time, 6,              "12:07");
    g_state.messages[1].is_mine = false;

    snprintf(g_state.messages[2].from, NODE_NAME_LEN, "MonNoeud");
    snprintf(g_state.messages[2].text, MSG_TEXT_LEN,  "Ok on arrive dans 10min");
    snprintf(g_state.messages[2].time, 6,              "12:10");
    g_state.messages[2].is_mine = true;
}

// Appelé toutes les 2s par le timer LVGL
// Retourne true si de nouvelles données sont arrivées
bool mesh_api_poll(void) {
    _mock_tick++;

    // Simule un nouveau message toutes les ~10s (5 ticks × 2s)
    if (_mock_tick % 5 == 0 && g_state.msg_count < MAX_MESSAGES) {
        int i = g_state.msg_count;

        if (_mock_tick == 5) {
            snprintf(g_state.messages[i].from, NODE_NAME_LEN, "Alice");
            snprintf(g_state.messages[i].text, MSG_TEXT_LEN,
                     "Le relay ne repond plus, vous le voyez ?");
            snprintf(g_state.messages[i].time, 6, "12:15");
            g_state.messages[i].is_mine = false;
        } else if (_mock_tick == 10) {
            snprintf(g_state.messages[i].from, NODE_NAME_LEN, "Bob");
            snprintf(g_state.messages[i].text, MSG_TEXT_LEN,
                     "Non, trop loin. RSSI -118 la derniere fois");
            snprintf(g_state.messages[i].time, 6, "12:16");
            g_state.messages[i].is_mine = false;
        } else {
            // message générique pour les tests longs
            snprintf(g_state.messages[i].from, NODE_NAME_LEN, "Alice");
            snprintf(g_state.messages[i].text, MSG_TEXT_LEN,
                     "Tick #%d — test message long distance LoRa 868MHz", _mock_tick);
            snprintf(g_state.messages[i].time, 6, "12:%02d", 17 + _mock_tick / 5);
            g_state.messages[i].is_mine = false;
        }
        g_state.msg_count++;
        return true;
    }

    return false;
}

bool mesh_api_send(const char *text) {
    if (!text || text[0] == '\0') return false;
    if (g_state.msg_count >= MAX_MESSAGES) return false;

    // TODO: POST http://localhost:4403/api/v1/toradio

    int i = g_state.msg_count;
    snprintf(g_state.messages[i].from, NODE_NAME_LEN, "%s", g_state.my_name);
    snprintf(g_state.messages[i].text, MSG_TEXT_LEN,  "%s", text);
    snprintf(g_state.messages[i].time, 6,              "envoi");
    g_state.messages[i].is_mine = true;
    g_state.msg_count++;

    return true;
}
