#include "app.h"
#include <string.h>
#include <stdio.h>

// ─────────────────────────────────────────────────────────────────────────────
//  screen_messages.c  —  Écran principal : chat Meshtastic
//
//  Layout 320×170 :
//  ┌────────────────────────────────────────────────────────┐  18px  statusbar
//  │ 📡 Meshtastic   LongFast   EU_868   [3 nœuds]   ● OK  │
//  ├────────────────────────────────────────────────────────┤
//  │                                                        │  128px messages
//  │  12:03  Alice   Salut ! Vous etes combien ?            │  scrollable
//  │  12:07  Bob     Je suis au sommet, signal parfait      │
//  │  12:10  [Moi]   Ok on arrive dans 10min               │
//  │                                                        │
//  ├────────────────────────────────────────────────────────┤
//  │ > _                                            [N] [S] │  24px  input
//  └────────────────────────────────────────────────────────┘
//
//  Touches :
//   Entrée          → envoyer le message
//   Fn+N  (ou Tab)  → aller à l'écran Nœuds
//   Fn+S            → aller à l'écran Config
//   Flèches haut/bas → scroller les messages
// ─────────────────────────────────────────────────────────────────────────────

// Widgets persistants mis à jour par refresh
static lv_obj_t *_msg_list   = NULL;  // conteneur scrollable
static lv_obj_t *_input_ta   = NULL;  // zone de saisie
static lv_obj_t *_status_bar = NULL;

// Buffer de saisie clavier (géré à la main pour le clavier physique)
static char  _input_buf[MSG_TEXT_LEN] = {0};
static int   _input_len = 0;

// Référence vers l'écran parent (pour navigation)
static lv_obj_t *_root = NULL;

// Callback navigation (set par main.c)
static void (*_nav_cb)(ScreenId) = NULL;

// ─── Forward déclarations internes ───────────────────────────────────────────
static void _rebuild_message_list(void);
static void _append_message_label(const MeshMessage *msg);
static void _input_update_display(void);

// ─────────────────────────────────────────────────────────────────────────────
//  Styles statiques
// ─────────────────────────────────────────────────────────────────────────────

static lv_style_t _style_root;
static lv_style_t _style_statusbar;
static lv_style_t _style_msglist;
static lv_style_t _style_msg_mine;
static lv_style_t _style_msg_theirs;
static lv_style_t _style_input;
static bool       _styles_init = false;

static void _init_styles(void) {
    if (_styles_init) return;
    _styles_init = true;

    // Fond général
    lv_style_init(&_style_root);
    lv_style_set_bg_color(&_style_root, hex(COL_BG));
    lv_style_set_bg_opa(&_style_root, LV_OPA_COVER);
    lv_style_set_border_width(&_style_root, 0);
    lv_style_set_pad_all(&_style_root, 0);

    // Barre de statut
    lv_style_init(&_style_statusbar);
    lv_style_set_bg_color(&_style_statusbar, hex(COL_SURFACE));
    lv_style_set_bg_opa(&_style_statusbar, LV_OPA_COVER);
    lv_style_set_border_width(&_style_statusbar, 0);
    lv_style_set_border_side(&_style_statusbar, LV_BORDER_SIDE_BOTTOM);
    lv_style_set_border_color(&_style_statusbar, hex(COL_BORDER));
    lv_style_set_pad_left(&_style_statusbar, 6);
    lv_style_set_pad_right(&_style_statusbar, 6);
    lv_style_set_pad_top(&_style_statusbar, 2);
    lv_style_set_pad_bottom(&_style_statusbar, 2);

    // Liste de messages scrollable
    lv_style_init(&_style_msglist);
    lv_style_set_bg_color(&_style_msglist, hex(COL_BG));
    lv_style_set_bg_opa(&_style_msglist, LV_OPA_COVER);
    lv_style_set_border_width(&_style_msglist, 0);
    lv_style_set_pad_left(&_style_msglist, 4);
    lv_style_set_pad_right(&_style_msglist, 4);
    lv_style_set_pad_top(&_style_msglist, 2);
    lv_style_set_pad_bottom(&_style_msglist, 2);
    lv_style_set_pad_row(&_style_msglist, 1);

    // Messages des autres
    lv_style_init(&_style_msg_theirs);
    lv_style_set_text_color(&_style_msg_theirs, hex(COL_WHITE));
    lv_style_set_text_font(&_style_msg_theirs, &lv_font_montserrat_10);
    lv_style_set_bg_opa(&_style_msg_theirs, LV_OPA_TRANSP);
    lv_style_set_border_width(&_style_msg_theirs, 0);
    lv_style_set_pad_all(&_style_msg_theirs, 0);
    lv_style_set_width(&_style_msg_theirs, LV_PCT(100));

    // Nos propres messages (en vert)
    lv_style_init(&_style_msg_mine);
    lv_style_set_text_color(&_style_msg_mine, hex(COL_GREEN));
    lv_style_set_text_font(&_style_msg_mine, &lv_font_montserrat_10);
    lv_style_set_bg_opa(&_style_msg_mine, LV_OPA_TRANSP);
    lv_style_set_border_width(&_style_msg_mine, 0);
    lv_style_set_pad_all(&_style_msg_mine, 0);
    lv_style_set_width(&_style_msg_mine, LV_PCT(100));

    // Zone de saisie
    lv_style_init(&_style_input);
    lv_style_set_bg_color(&_style_input, hex(COL_SURFACE));
    lv_style_set_bg_opa(&_style_input, LV_OPA_COVER);
    lv_style_set_border_width(&_style_input, 0);
    lv_style_set_border_side(&_style_input, LV_BORDER_SIDE_TOP);
    lv_style_set_border_color(&_style_input, hex(COL_BORDER));
    lv_style_set_text_color(&_style_input, hex(COL_WHITE));
    lv_style_set_text_font(&_style_input, &lv_font_montserrat_10);
    lv_style_set_pad_left(&_style_input, 6);
    lv_style_set_pad_top(&_style_input, 4);
    lv_style_set_pad_bottom(&_style_input, 4);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Création de l'écran
// ─────────────────────────────────────────────────────────────────────────────

lv_obj_t *screen_messages_create(lv_obj_t *parent) {
    _init_styles();

    // Conteneur racine plein écran
    _root = lv_obj_create(parent);
    lv_obj_set_size(_root, CZ_W, CZ_H);
    lv_obj_set_pos(_root, 0, 0);
    lv_obj_add_style(_root, &_style_root, 0);
    lv_obj_set_scrollbar_mode(_root, LV_SCROLLBAR_MODE_OFF);

    // ── Barre de statut (haut, 18px) ─────────────────────────────────────────
    _status_bar = lv_label_create(_root);
    lv_obj_set_size(_status_bar, CZ_W, STATUSBAR_H);
    lv_obj_set_pos(_status_bar, 0, 0);
    lv_obj_add_style(_status_bar, &_style_statusbar, 0);
    lv_label_set_text(_status_bar, "");

    // ── Liste de messages (zone centrale scrollable) ───────────────────────────
    _msg_list = lv_obj_create(_root);
    lv_obj_set_size(_msg_list, CZ_W, CONTENT_H);
    lv_obj_set_pos(_msg_list, 0, STATUSBAR_H);
    lv_obj_add_style(_msg_list, &_style_msglist, 0);
    lv_obj_set_flex_flow(_msg_list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scroll_dir(_msg_list, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(_msg_list, LV_SCROLLBAR_MODE_OFF);

    // ── Zone de saisie (bas, 24px) ────────────────────────────────────────────
    _input_ta = lv_label_create(_root);
    lv_obj_set_size(_input_ta, CZ_W, INPUT_H);
    lv_obj_set_pos(_input_ta, 0, STATUSBAR_H + CONTENT_H);
    lv_obj_add_style(_input_ta, &_style_input, 0);
    lv_label_set_text(_input_ta, "> _");

    // Remplir avec les données initiales
    screen_messages_refresh();

    return _root;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Refresh — reconstruit la liste et la statusbar
// ─────────────────────────────────────────────────────────────────────────────

void screen_messages_refresh(void) {
    if (!_msg_list) return;

    // Mettre à jour la statusbar
    char status[96];
    snprintf(status, sizeof(status),
             "📡 %s  |  %s  %s  |  %d noeud%s  %s",
             g_state.my_name,
             g_state.channel,
             g_state.region,
             g_state.node_count,
             g_state.node_count > 1 ? "s" : "",
             g_state.daemon_connected ? "● OK" : "○ --");
    lv_label_set_text(_status_bar, status);

    // Reconstruire la liste de messages
    _rebuild_message_list();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Entrées clavier physique
// ─────────────────────────────────────────────────────────────────────────────

void screen_messages_on_key(uint32_t key) {
    if (!_input_ta) return;

    if (key == LV_KEY_ENTER) {
        // ── Envoyer le message ────────────────────────────────────────────────
        if (_input_len > 0) {
            _input_buf[_input_len] = '\0';
            mesh_api_send(_input_buf);
            _input_len = 0;
            _input_buf[0] = '\0';
            screen_messages_refresh();
        }

    } else if (key == LV_KEY_BACKSPACE || key == 8) {
        // ── Effacer un caractère ──────────────────────────────────────────────
        if (_input_len > 0) {
            _input_len--;
            _input_buf[_input_len] = '\0';
        }

    } else if (key == LV_KEY_UP) {
        // ── Scroller vers le haut ─────────────────────────────────────────────
        lv_obj_scroll_by(_msg_list, 0, -20, LV_ANIM_ON);
        return;

    } else if (key == LV_KEY_DOWN) {
        // ── Scroller vers le bas ──────────────────────────────────────────────
        lv_obj_scroll_by(_msg_list, 0, 20, LV_ANIM_ON);
        return;

    } else if (key >= 0x20 && key < 0x7F) {
        // ── Caractère imprimable ──────────────────────────────────────────────
        if (_input_len < MSG_TEXT_LEN - 1) {
            _input_buf[_input_len++] = (char)key;
            _input_buf[_input_len]   = '\0';
        }
    }

    _input_update_display();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Helpers internes
// ─────────────────────────────────────────────────────────────────────────────

static void _append_message_label(const MeshMessage *msg) {
    // Format : "HH:MM  Nom  texte…"
    // Les messages de l'utilisateur sont en vert, les autres en blanc
    char buf[MSG_TEXT_LEN + NODE_NAME_LEN + 16];
    snprintf(buf, sizeof(buf), "%s  %-10s  %s",
             msg->time, msg->from, msg->text);

    lv_obj_t *label = lv_label_create(_msg_list);
    lv_label_set_text(label, buf);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(label, CZ_W - 8);

    if (msg->is_mine) {
        lv_obj_add_style(label, &_style_msg_mine, 0);
    } else {
        lv_obj_add_style(label, &_style_msg_theirs, 0);
    }
}

static void _rebuild_message_list(void) {
    // Supprimer tous les anciens labels
    lv_obj_clean(_msg_list);

    // Re-créer tous les messages
    for (int i = 0; i < g_state.msg_count; i++) {
        _append_message_label(&g_state.messages[i]);
    }

    // Auto-scroll vers le bas pour voir le dernier message
    lv_obj_scroll_to_y(_msg_list, LV_COORD_MAX, LV_ANIM_OFF);
}

static void _input_update_display(void) {
    char display[MSG_TEXT_LEN + 8];
    snprintf(display, sizeof(display), "> %s_", _input_buf);
    lv_label_set_text(_input_ta, display);
}
