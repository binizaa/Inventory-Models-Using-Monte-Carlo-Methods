#ifndef MENU_H
#define MENU_H

#include <string>

#include "analysis/eoq.h"

// ---------------------------------------------------------------------------
// Entrada del usuario
// ---------------------------------------------------------------------------

// Lee un numero desde la consola, repitiendo la pregunta si no es valido
double readValue(const std::string &prompt);

enum MenuOption { DEFAULT_EXAMPLE = 1, MANUAL_INPUT = 2 };

// Muestra el menu y devuelve la opcion elegida
MenuOption askMenuOption();

// Pide S, H, L, nivel de servicio e inventario inicial
InventoryParams readParams();

void printUserParams(const InventoryParams &p);

#endif
