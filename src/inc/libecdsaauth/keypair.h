/*
 * Copyright (c) 2013 William Pitcock <nenolod@dereferenced.org>.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef KEYPAIR_H
#define KEYPAIR_H

#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

#include "base64.h"

typedef struct libecdsaauth_key_s {
	EC_KEY *eckey;
} libecdsaauth_key_t;

/*
 * Generate an ECC keypair suitable for authentication.
 */
extern libecdsaauth_key_t *libecdsaauth_key_new(void);

/*
 * Load an ECC keypair from a PEM file.
 */
extern libecdsaauth_key_t *libecdsaauth_key_load(const char *filename);

/*
 * Deserialize a raw public key.
 */
extern libecdsaauth_key_t *libecdsaauth_key_from_pubkey(unsigned char *pubkey_raw, size_t len);

/*
 * Deserialize a public key in base64 encapsulation.
 */
extern libecdsaauth_key_t *libecdsaauth_key_from_base64_pubkey(const char *keydata);

/*
 * Free an ECC key.
 */
extern void libecdsaauth_key_free(libecdsaauth_key_t *key);

/*
 * Return public key component length, if it were a binary blob.
 */
extern size_t libecdsaauth_key_public_key_length(libecdsaauth_key_t *key);

/*
 * Return public key as binary blob.  Use libecdsaauth_key_public_key_length() to
 * get the key length in bytes.
 */
extern unsigned char *libecdsaauth_key_public_key_blob(libecdsaauth_key_t *key);

/*
 * Return public key as base64 blob.
 */
extern char *libecdsaauth_key_public_key_base64(libecdsaauth_key_t *key);

#endif
