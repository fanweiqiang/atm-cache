/*
 * =====================================================================================
 *
 *       Filename:  server.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  03/28/2017 09:40:41 PM
 *       Compiler:  gcc
 *
 *         Author:  JUNTAO DUAN (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <atm_config.h>
#include <atm_core.h>

static void atm_initialize() {
    atm_hash_init_siphash_seed();
}

static void atm_service() {

}

int main(int argc, char **argv) {
    atm_initialize();
    atm_service();
    return 0;
}

