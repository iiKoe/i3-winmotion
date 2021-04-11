#include <assert.h>
#include <stdio.h>
#include <cstdint>
#include <iostream>
#include <optional>
#include <list>
#include <set>
#include <map>
#include <cmath>

#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xutil.h>

#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>

#include "i3_ipc.hpp"
#include "i3_containers.hpp"

#include "Config.h"
#include "printtree.h"

using namespace std;


const i3_containers::node *find_workspace_root(const i3_containers::node *node, const string &visible_workspace_name) {
    if (node == nullptr) {
        return nullptr;
    }

    if ((node->type == i3_containers::node_type::workspace) && (node->name.value_or("") == visible_workspace_name)) {
        return node;
    }

    for (const i3_containers::node &child : node->nodes) {
        const auto * vis = find_workspace_root(&child, visible_workspace_name);
        if (vis != nullptr) {
            return vis;
        }
    }
    return nullptr;
}

void find_visible_nodes(const i3_containers::node &node,
        list<const i3_containers::node *> &visible_nodes ) {

    // if this is a leaf node we add it and return
    if (node.nodes.size() == 0 && node.type == i3_containers::node_type::con) {
        visible_nodes.push_back(&node);
    }

    // find the nodes for the visible workspace
    switch (node.layout) {
        case i3_containers::node_layout::tabbed:
        case i3_containers::node_layout::stacked:
            if (node.focus.size() > 0) {
                auto focus_id = node.focus.front();
                for (const auto &child : node.nodes) {
                    if (focus_id == child.id)
                        find_visible_nodes(child, visible_nodes);
                }
            }
            break;
        default:
            for (const auto &child : node.nodes) {
                find_visible_nodes(child, visible_nodes);
            }
            break;
    }
}


void draw(cairo_t *cr, int width, int height) {
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_fill(cr);
}

void draw_rect(cairo_t *cr, double x, double y, double w, double h, int r) {
    cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);
    x -= w/2.0;
    y -= h/2.0;

    cairo_new_sub_path(cr);
    cairo_arc(cr, x + r, y + r, r, M_PI, 3 * M_PI / 2);
    cairo_arc(cr, x + w - r, y + r, r, 3 *M_PI / 2, 2 * M_PI);
    cairo_arc(cr, x + w - r, y + h - r, r, 0, M_PI / 2);
    cairo_arc(cr, x + r, y + h - r, r, M_PI / 2, M_PI);
    cairo_close_path(cr);
    cairo_fill(cr);
}

void draw_text(cairo_t *cr, double x, double y, string &text) {
    cairo_text_extents_t extents;

    cairo_text_extents(cr, text.c_str(), &extents);

    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.6);
    draw_rect(cr, x, y, extents.width*3, extents.height*3, 20);

    double x_text = x - (extents.width/2 + extents.x_bearing);
    double y_text = y - (extents.height/2 + extents.y_bearing);

    cairo_set_source_rgba(cr, 255, 255, 255, 1);
    cairo_move_to(cr, x_text, y_text);
    cairo_show_text(cr, text.c_str());

}

void construct_hints(list<string> &hints, string hint, list<char> &hint_keys, int x) {
    if (x == 1) {
        for (auto h : hint_keys) {
            hints.push_back(hint+h);
        }
    } else {
        for (auto h : hint_keys) {
            construct_hints(hints, hint+h, hint_keys, x-1);
        }
    }
}

/*
 L = number of hint characters
 n = number of windows needing hints
 x = number of hint characters needed per hint
 */
int get_hints(list<string> &hints, list<char> &hint_keys, unsigned int n) {
    double L = hint_keys.size();
    int x;

    assert(L>0);

    if (L <= 1)
        x = n;
    else
        x = ceil(log10((double)n)/log10(L));

    construct_hints(hints, "", hint_keys, x);
    return x;
}

int main(int argc, char *argv[]) {
    Config config = {argc, argv};
    config.print();
    //exit(1);

    // Create IPC object and connect it to running i3 process.
    i3_ipc i3;

    // Get internal node tree from i3.
    const i3_containers::node tree = i3.get_tree();

    auto workspaces = i3.get_workspaces();
    list<string> visible_workspace_names;

    for (const auto &w : workspaces) {
        if (w.is_visible) {
            visible_workspace_names.push_back(w.name);
        }
    }

    list<const i3_containers::node *> visible_nodes;

    for (const auto &visible_workspace_name: visible_workspace_names) {
        const auto *workspace_root = find_workspace_root(&tree, visible_workspace_name);
        if (workspace_root == nullptr) {
            cout << "Root workspace not found in the tree" << endl;
            return 1;
        }
        //print_i3_tree(*workspace_root);

        find_visible_nodes(*workspace_root, visible_nodes);

        // Optionally find floating nodes (all)
        if (config.hint_floating) {
            for (const auto &fnode : workspace_root->floating_nodes) {
                for (const auto &fchild : fnode.nodes) {
                    visible_nodes.push_back(&fchild);
                }
            }
        }
    }

    cout << "Visible nodes:" << endl;
    for (const auto *node : visible_nodes) {
        cout << node->name.value_or("unknown") << endl;
    }

    if (visible_nodes.size() <= 1) {
        cout << "not enough visible nodes" << endl;
        exit(EXIT_SUCCESS);
    }

    // draw the overlays
    Display *d = XOpenDisplay(NULL);
    Window root = DefaultRootWindow(d);
    int default_screen = XDefaultScreen(d);

    // these two lines are really all you need
    XSetWindowAttributes attrs;
    attrs.override_redirect = true;

    XVisualInfo vinfo;
    if (!XMatchVisualInfo(d, DefaultScreen(d), 32, TrueColor, &vinfo)) {
        printf("No visual found supporting 32 bit color, terminating\n");
        exit(EXIT_FAILURE);
    }

    auto width = DisplayWidth(d, default_screen);
    auto height = DisplayHeight(d, default_screen);
    cout << "Screen width: " << width << " height:" << height << endl;

    // these next three lines add 32 bit depth, remove if you dont need and change the flags below
    attrs.colormap = XCreateColormap(d, root, vinfo.visual, AllocNone);
    attrs.background_pixel = 0;
    attrs.border_pixel = 0;
    attrs.event_mask = KeyPressMask | KeyReleaseMask;

    // Window XCreateWindow(
    //     Display *display, Window parent,
    //     int x, int y, unsigned int width, unsigned int height, unsigned int border_width,
    //     int depth, unsigned int class,
    //     Visual *visual,
    //     unsigned long valuemask, XSetWindowAttributes *attributes
    // );
    Window overlay = XCreateWindow(
        d, root,
        0, 0, width, height, 0,
        vinfo.depth, InputOutput,
        vinfo.visual,
        CWOverrideRedirect | CWEventMask | CWColormap | CWBackPixel | CWBorderPixel, &attrs
    );

    /* select kind of events we are interested in */
    XSelectInput(d, overlay, KeyPressMask | KeyReleaseMask );

    XMapWindow(d, overlay);

    cairo_surface_t* surf = cairo_xlib_surface_create(d, overlay,
                                  vinfo.visual,
                                  width, height);
    cairo_t* cr = cairo_create(surf);

    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.4);
    if (config.fullscreen_overlay) {
        draw(cr, width, height);
    }

    /* Draw some text */
    cairo_select_font_face(cr, config.font.c_str(), CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, config.font_size);

    // Generate hints
    list<string> hints;
    list<char> &hint_keys = config.hint_keys;
    unsigned int keys_in_hint = get_hints(hints, hint_keys, visible_nodes.size());

#if 0
    cout << "Key combos" << endl;
    for (const auto &combo : hints) {
        cout << "Key combo: " << combo << endl;
    }
#endif

    map<string, const i3_containers::node *> key_node_map;

    for (const auto *node : visible_nodes) {
        string hint = hints.front();
        hints.pop_front();

        key_node_map[hint] = node;

        auto rect = node->rect;
        double x = rect.x + rect.width/2.0;
        double y = rect.y + rect.height/2.0;

        for (auto &c: hint) c = toupper(c);
        draw_text(cr, x, y, hint);
    }

    XFlush(d);

    // Take the focus
    XSetInputFocus(d, overlay, RevertToNone, CurrentTime);

    //this_thread::sleep_for(chrono::milliseconds(5000));
    /* event loop */
    cout << "Entering event loop" << endl;
    XEvent event;

    string key_string;
    while (1) {
        XNextEvent(d, &event);

        /* keyboard events */
        if (event.type == KeyPress) {
            //printf( "KeyPress: %x\n", event.xkey.keycode);
            /* exit on ESC key press */
            if ( event.xkey.keycode == 0x09 )
                break;

            char buff[32];
            XLookupString(&event.xkey, buff, sizeof(buff), NULL, NULL);
            string c = string(buff);
            cout << "Keypress: " << c << endl;

            key_string += c;

            if (std::find(std::begin(hint_keys), std::end(hint_keys), buff[0]) == std::end(hint_keys)) {
                cout << "key '" << c << "' is not a valid hint key" << endl;
                break;
            }

            // If enough keys where collected
            if (key_string.size() == keys_in_hint)
                break;
        }
    }

    cairo_destroy(cr);
    cairo_surface_destroy(surf);

    XUnmapWindow(d, overlay);

    auto selected = key_node_map.find(key_string);
    if (selected != key_node_map.end()) {
        // Found a valid hint
        auto focus_node = selected->second;
        cout << "Focussing on '" << focus_node->name.value_or("unknown") << "'" << endl;
        if (focus_node->window.has_value()) {
            auto focus_window = focus_node->window.value();
            XSetInputFocus(d, focus_window, RevertToParent, CurrentTime);
        } else {
            cout << "Focus error" << endl;
        }
    } else {
        cout << "Unrecognised hint: '" << key_string << "'" << endl;
    }

    XCloseDisplay(d);

    return EXIT_SUCCESS;
}

