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

#include "libecdsaauth/keypair.h"

#include <string.h>
#include <stdlib.h>

static inline libecdsaauth_key_t *libecdsaauth_key_alloc(void)
{
	libecdsaauth_key_t *key = calloc(sizeof(libecdsaauth_key_t), 1);

	key->eckey = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
	EC_KEY_set_conv_form(key->eckey, POINT_CONVERSION_COMPRESSED);

	return key;
}

/*
 * Generate an ECC keypair suitable for authentication.
 */
libecdsaauth_key_t *libecdsaauth_key_new(void)
{
	libecdsaauth_key_t *key = libecdsaauth_key_alloc();

	if (key->eckey != NULL)
		EC_KEY_generate_key(key->eckey);
	else
	{
		libecdsaauth_key_free(key);
		return NULL;
	}

	return key;
}

/*
 * Load an ECC keypair from a PEM file.
 */
libecdsaauth_key_t *libecdsaauth_key_load(const char *filename)
{
	libecdsaauth_key_t *key = libecdsaauth_key_alloc();
	FILE *in;

	in = fopen(filename, "r");
	if (in == NULL)
	{
		libecdsaauth_key_free(key);
		return NULL;
	}

	PEM_read_ECPrivateKey(in, &key->eckey, NULL, NULL);
	fclose(in);

	EC_KEY_set_conv_form(key->eckey, POINT_CONVERSION_COMPRESSED);
	if (!EC_KEY_check_key(key->eckey))
	{
		libecdsaauth_key_free(key);
		return NULL;
	}

	return key;
}

/*
 * Deserialize a raw public key.
 */
libecdsaauth_key_t *libecdsaauth_key_from_pubkey(unsigned char *pubkey_raw, size_t len)
{
	libecdsaauth_key_t *key = libecdsaauth_key_alloc();
	const unsigned char *pubkey_raw_p = pubkey_raw;

	o2i_ECPublicKey(&key->eckey, &pubkey_raw_p, len);
	if (!EC_KEY_check_key(key->eckey))
	{
		libecdsaauth_key_free(key);
		return NULL;
	}

	return key;
}

/*
 * Deserialize a public key in base64 encapsulation.
 */
libecdsaauth_key_t *libecdsaauth_key_from_base64_pubkey(const char *keydata)
{
	char workbuf[2048];
	size_t len;

	len = base64_decode(keydata, workbuf, sizeof workbuf);
	if (len == -1)
		return NULL;

	return libecdsaauth_key_from_pubkey(workbuf, len);
}

/*
 * Free an ECC key.
 */
void libecdsaauth_key_free(libecdsaauth_key_t *key)
{
	if (key->eckey != NULL)
		EC_KEY_free(key->eckey);

	free(key);
}

/*
 * Return public key component length, if it were a binary blob.
 */
size_t libecdsaauth_key_public_key_length(libecdsaauth_key_t *key)
{
	if (key->eckey == NULL)
		return 0;

	return (size_t) i2o_ECPublicKey(key->eckey, NULL);
}

/*
 * Return public key as binary blob.  Use libecdsaauth_key_public_key_length() to
 * get the key length in bytes.
 */
unsigned char *libecdsaauth_key_public_key_blob(libecdsaauth_key_t *key)
{
	unsigned char *keybuf, *keybuf_p;
	size_t len;

	if (key->eckey == NULL)
		return NULL;

	len = libecdsaauth_key_public_key_length(key);
	keybuf = malloc(len);
	keybuf_p = keybuf;

	i2o_ECPublicKey(key->eckey, &keybuf_p);
	keybuf_p = keybuf;

	return keybuf_p;
}

/*
 * Return public key as base64 blob.
 */
char *libecdsaauth_key_public_key_base64(libecdsaauth_key_t *key)
{
	unsigned char *keybuf;
	size_t keylen;
	char b64buf[1024];

	if (key->eckey == NULL)
		return NULL;

	memset(b64buf, 0, sizeof b64buf);

	keylen = libecdsaauth_key_public_key_length(key);
	keybuf = libecdsaauth_key_public_key_blob(key);
	base64_encode(keybuf, keylen, b64buf, sizeof b64buf);

	free(keybuf);

	return strdup(b64buf);
}
