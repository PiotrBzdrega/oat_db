#include <openssl/evp.h>
#include <openssl/err.h>
#include <cstdio>
#include <string.h>
#include <memory>

void handleErrors() {
    ERR_print_errors_fp(stderr);
    abort();
}

void sha(const char* input,uint8_t *hash)
{
    OSSL_LIB_CTX *library_context;
    int ret = 0;
    const char *option_properties = NULL;
    EVP_MD *message_digest = NULL;
    EVP_MD_CTX *digest_context = NULL;
    unsigned int digest_length;
    unsigned char *digest_value = NULL;
    unsigned int j;

    // //TODO: add deleter
    // auto file = std::unique_ptr<FILE>(
    //     fopen("/home/piotrbz/selkie-supervisor/test/Containerfile.yaml", "rb"));

    //    // Buffer to hold file data
    // const size_t buffer_size = 4096; // Read in chunks of 4 KB
    // unsigned char buffer[buffer_size];
    // size_t bytes_read;


    library_context = OSSL_LIB_CTX_new();
    if (library_context == NULL) {
        fprintf(stderr, "OSSL_LIB_CTX_new() returned NULL\n");
        goto cleanup;
    }

    /*
     * Fetch a message digest by name
     * The algorithm name is case insensitive.
     * See providers(7) for details about algorithm fetching
     */
    message_digest = EVP_MD_fetch(library_context,
                                  "SHA-256", option_properties);
    if (message_digest == NULL) {
        fprintf(stderr, "EVP_MD_fetch could not find SHA-256.");
        goto cleanup;
    }
    /* Determine the length of the fetched digest type */
    digest_length = EVP_MD_get_size(message_digest);
    if (digest_length <= 0) {
        fprintf(stderr, "EVP_MD_get_size returned invalid size.\n");
        goto cleanup;
    }

    digest_value =(unsigned char*) OPENSSL_malloc(digest_length);
    if (digest_value == NULL) {
        fprintf(stderr, "No memory.\n");
        goto cleanup;
    }
    /*
     * Make a message digest context to hold temporary state
     * during digest creation
     */
    digest_context = EVP_MD_CTX_new();
    if (digest_context == NULL) {
        fprintf(stderr, "EVP_MD_CTX_new failed.\n");
        goto cleanup;
    }
    /*
     * Initialize the message digest context to use the fetched
     * digest provider
     */
    if (EVP_DigestInit(digest_context, message_digest) != 1) {
        fprintf(stderr, "EVP_DigestInit failed.\n");
        goto cleanup;
    }
    // /* Digest parts one and two of the soliloqy */
    // if (EVP_DigestUpdate(digest_context, hamlet_1, strlen(hamlet_1)) != 1) {
    //     fprintf(stderr, "EVP_DigestUpdate(hamlet_1) failed.\n");
    //     goto cleanup;
    // }

    if (1 != EVP_DigestUpdate(digest_context, input, strlen(input)))
    {
        fprintf(stderr, "EVP_DigestUpdate(hamlet_1) failed.\n");
        goto cleanup;
    }

    if (EVP_DigestFinal(digest_context, digest_value, &digest_length) != 1) {
        fprintf(stderr, "EVP_DigestFinal() failed.\n");
        goto cleanup;
    }
    for (j=0; j<digest_length; j++)  {
        fprintf(stdout, "%02x", digest_value[j]);
    }
    fprintf(stdout, "\n");
    // /* Check digest_value against the known answer */
    // if ((size_t)digest_length != sizeof(known_answer)) {
    //     fprintf(stdout, "Digest length(%d) not equal to known answer length(%lu).\n",
    //         digest_length, sizeof(known_answer));
    // } else if (memcmp(digest_value, known_answer, digest_length) != 0) {
    //     for (j=0; j<sizeof(known_answer); j++) {
    //         fprintf(stdout, "%02x", known_answer[j] );
    //     }
    //     fprintf(stdout, "\nDigest does not match known answer\n");
    // } else {
    //     fprintf(stdout, "Digest computed properly.\n");
    //     ret = 1;
    // }

cleanup:
    if (ret != 1)
        ERR_print_errors_fp(stderr);
    /* OpenSSL free functions will ignore NULL arguments */
    EVP_MD_CTX_free(digest_context);
    OPENSSL_free(digest_value);
    EVP_MD_free(message_digest);

    OSSL_LIB_CTX_free(library_context);
}