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

#include "libecdsaauth/op.h"

#include <string.h>

/*
 * Verify a signature.
 */
bool libecdsaauth_verify(libecdsaauth_key_t *key, unsigned char *blob, size_t len, unsigned char *sig, size_t siglen)
{
	if (1 != ECDSA_verify(0, blob, len, sig, siglen, key->eckey))
		return false;

	return true;
}

/*
 * Verify a base64-encapsulated signature.
 */
bool libecdsaauth_verify_base64(libecdsaauth_key_t *key, unsigned char *blob, size_t len, char *sig_base64)
{
	unsigned char sigbuf[1024];
	size_t siglen;

	siglen = base64_decode(sig_base64, sigbuf, sizeof sigbuf);

	return libecdsaauth_verify(key, blob, len, sigbuf, siglen);
}

/*
 * Sign a challenge.
 */
bool libecdsaauth_sign(libecdsaauth_key_t *key, unsigned char *in, size_t inlen, unsigned char **out, size_t *outlen)
{
	unsigned char *sig_buf, *sig_buf_p;
	unsigned int sig_len;

	if (key->eckey == NULL)
		return false;

	sig_len = ECDSA_size(key->eckey);
	sig_buf = malloc(sig_len);
	sig_buf_p = sig_buf;

	if (!ECDSA_sign(0, in, inlen, sig_buf_p, &sig_len, key->eckey))
	{
		free(sig_buf);
		return false;
	}

	*out = sig_buf;
	*outlen = (size_t) sig_len;

	return true;
}

/*
 * Sign a challenge and base64 it.
 */
bool libecdsaauth_sign_base64(libecdsaauth_key_t *key, unsigned char *in, size_t inlen, char **out, size_t *outlen)
{
	char buf[1024], inbuf[1024];
	unsigned char *workbuf_p, *outbuf_p;
	size_t len, buflen;

	memset(inbuf, 0, sizeof inbuf);
	len = base64_decode(in, inbuf, sizeof inbuf);
	workbuf_p = (unsigned char *) inbuf;

	outbuf_p = NULL;

	if (!libecdsaauth_sign(key, inbuf, len, &outbuf_p, &buflen))
		return false;

	if (outbuf_p == NULL)
		return false;

	memset(buf, 0, sizeof buf);
	len = base64_encode(outbuf_p, buflen, buf, sizeof buf);
	free(outbuf_p);

	*out = strdup(buf);
	*outlen = len;

	return true;
}

/*******************************************************************************************/

/*
 * Create a new challenge for a key.
 * Returns an object for tracking the challenge.
 */
libecdsaauth_challenge_t *libecdsaauth_challenge_new(libecdsaauth_key_t *key)
{
	libecdsaauth_challenge_t *challenge = calloc(sizeof(libecdsaauth_challenge_t), 1);

	challenge->key = key;
	RAND_pseudo_bytes(challenge->blob, sizeof(challenge->blob));

	return challenge;
}

/*
 * Frees a challenge.
 */
void libecdsaauth_challenge_free(libecdsaauth_challenge_t *challenge)
{
	free(challenge);
}

/*
 * Returns the challenge bytes as a binary blob.
 */
unsigned char *libecdsaauth_challenge_bytes(libecdsaauth_challenge_t *challenge)
{
	if (challenge->key == NULL)
		return NULL;

	return challenge->blob;
}

/*
 * Returns the size of the challenge payload.
 */
size_t libecdsaauth_challenge_size(libecdsaauth_challenge_t *challenge)
{
	return sizeof(challenge->blob);
}

/*
 * Verify a challenge against a blob.
 */
bool libecdsaauth_challenge_verify(libecdsaauth_challenge_t *challenge, unsigned char *blob, size_t len)
{
	return libecdsaauth_verify(challenge->key, challenge->blob, sizeof(challenge->blob), blob, len);
}

/*
 * Verify a challenge against a base64 blob.
 */
bool libecdsaauth_challenge_verify_base64(libecdsaauth_challenge_t *challenge, char *blob_base64)
{
	unsigned char blob[1024];
	size_t bloblen;

	bloblen = base64_decode(blob_base64, blob, sizeof blob);

	return libecdsaauth_challenge_verify(challenge, blob, bloblen);
}

