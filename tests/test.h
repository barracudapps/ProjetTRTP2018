#include <zlib.h>
#include "../src/packet_interface.h"
#include "../src/network.h"

#define HEADER_SIZE 12

/* Si l'option -v est spepcifiee, verbose est mis a 1 et le programme affiche
 * tous les tests. Sinon seulement les tests ayant echoue sont affiches.
 */
int verbose = 0;

// compteurs de tests.
int passed = 0;
int total = 0;

/*
 * Fonctions de test servant de base à toutes les autre fonctions de test.
 */
void test(int expected, int actual, const char* name);

void test_unexpected(int unexpected, int actual, const char* name);

void test_ptr(void* expected, void* actual, const char* name);

void test_unexpected_ptr(void* expected, void* actual, const char* name);

// Fonctions
void test_pkt();

void test_window();

void test_network();
