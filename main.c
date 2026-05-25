#include "app.h"
#include <string.h>

// ─────────────────────────────────────────────────────────────────────────────
//  main.c  —  Point d'entrée de l'app Meshtastic pour CardputerZero
//
//  Gère :
//   - l'initialisation de l'API mock (→ vraie plus tard)
//   - la création des 3 écrans
//   - la navigation clavier entre écrans (Fn+M / Fn+N / Fn+S)
//   - le timer de polling meshtasticd toutes les 2s
// ─────────────────────────────────────────────────────────────────────────────

// ─── État navigation ──────────────────────────────────────────────────────────
static ScreenId    _current = SCREEN_MESSAGES;
static lv_obj_t   *_screens[SCREEN_COUNT];
static lv_obj_t   *_root_parent = NULL;

// ─── Touches Fn (combinaisons clavier CardputerZero) ─────────────────────────
// Le clavier du CZ envoie des keycodes ; on garde une touche Fn pressée en état
static bool _fn_held = false;

// ─── Prototypes internes ──────────────────────────────────────────────────────
static void _show_screen(ScreenId id);
static void _poll_timer_cb(lv_timer_t *t);

// ─────────────────────────────────────────────────────────────────────────────
//  app_main — appelé par le shell APPLaunch au démarrage
// ─────────────────────────────────────────────────────────────────────────────
void app_main(lv_obj_t *parent) {
    _root_parent = parent;

    // 1. Initialiser l'API (mock pour l'instant)
    mesh_api_init();

    // 2. Créer les 3 écrans (tous cachés sauf le premier)
    _screens[SCREEN_MESSAGES] = screen_messages_create(parent);
    _screens[SCREEN_NODES]    = screen_nodes_create(parent);
    _screens[SCREEN_CONFIG]   = screen_config_create(parent);

    // 3. Afficher l'écran Messages par défaut
    _show_screen(SCREEN_MESSAGES);

    // 4. Timer polling meshtasticd toutes les 2000ms
    lv_timer_create(_poll_timer_cb, 2000, NULL);
}

// ─────────────────────────────────────────────────────────────────────────────
//  app_event — appelé à chaque touche du clavier physique
// ─────────────────────────────────────────────────────────────────────────────
void app_event(int type, void *data) {
    // czdev passe les keycodes LVGL via LV_EVENT_KEY
    if (type != LV_EVENT_KEY) return;

    uint32_t key = *(uint32_t *)data;

    // ── Touche Fn (keycode 0x01 sur CardputerZero) ────────────────────────────
    // Adapter selon le keycode réel une fois le device en main
    if (key == 0x01) {
        _fn_held = !_fn_held;
        return;
    }

    // ── Combos Fn+lettre : navigation entre écrans ────────────────────────────
    if (_fn_held) {
        _fn_held = false;  // consomme le Fn

        switch (key) {
            case 'm': case 'M':
                _show_screen(SCREEN_MESSAGES); return;
            case 'n': case 'N':
                _show_screen(SCREEN_NODES);   return;
            case 's': case 'S':
                _show_screen(SCREEN_CONFIG);  return;
            default:
                break;
        }
    }

    // ── Raccourcis sans Fn ────────────────────────────────────────────────────
    // Tab → basculer entre Messages et Nœuds
    if (key == LV_KEY_NEXT) {
        ScreenId next = (_current + 1) % SCREEN_COUNT;
        _show_screen(next);
        return;
    }

    // Escape → retour à Messages
    if (key == LV_KEY_ESC) {
        _show_screen(SCREEN_MESSAGES);
        return;
    }

    // ── Transmettre au gestionnaire de l'écran courant ────────────────────────
    switch (_current) {
        case SCREEN_MESSAGES:
            screen_messages_on_key(key); break;
        case SCREEN_NODES:
            screen_nodes_on_key(key);   break;
        case SCREEN_CONFIG:
            screen_config_on_key(key);  break;
        default:
            break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Helpers internes
// ─────────────────────────────────────────────────────────────────────────────

static void _show_screen(ScreenId id) {
    if (id >= SCREEN_COUNT) return;

    // Cacher tous les écrans
    for (int i = 0; i < SCREEN_COUNT; i++) {
        if (_screens[i]) lv_obj_add_flag(_screens[i], LV_OBJ_FLAG_HIDDEN);
    }

    // Afficher le bon
    if (_screens[id]) lv_obj_clear_flag(_screens[id], LV_OBJ_FLAG_HIDDEN);

    _current = id;
}

static void _poll_timer_cb(lv_timer_t *t) {
    (void)t;

    // Interroger l'API (mock ou vraie)
    bool fresh = mesh_api_poll();

    // Si nouvelles données → rafraîchir l'écran courant
    if (fresh) {
        switch (_current) {
            case SCREEN_MESSAGES:
                screen_messages_refresh(); break;
            case SCREEN_NODES:
                screen_nodes_refresh();   break;
            default:
                break;
        }
    }
}
