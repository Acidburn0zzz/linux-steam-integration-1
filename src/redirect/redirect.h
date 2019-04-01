/*
 * This file is part of linux-steam-integration.
 *
 * Copyright © 2019 Solus
 *
 * linux-steam-integration is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 */

#pragma once

#include <stdbool.h>
#include <stdlib.h>

/**
 * The type of redirect required
 */
typedef enum {
        LSI_REDIRECT_MIN = 1,
        LSI_REDIRECT_PATH,
} LsiRedirectType;

/**
 * LsiRedirect describes the configuration required for each operation.
 */
typedef struct LsiRedirect {
        LsiRedirectType type;
        union {
                /* Path replacement */
                struct {
                        char *path_source;
                        char *path_target;
                };
        };
        struct LsiRedirect *next;
} LsiRedirect;

/**
 * LsiRedirectOperation specifies the syscall we're intended for.
 */
typedef enum { LSI_OPERATION_OPEN = 0, LSI_NUM_OPERATIONS } LsiRedirectOperation;

/**
 * We build an LsiRedirectProfile to match the current process.
 * It contains a table of LsiRedirect members corresponding to a given
 * op, i.e:
 *
 *      op_table[LSI_OPERATION_OPEN]
 */
typedef struct LsiRedirectProfile {
        char *name; /**< Name for this profile */

        LsiRedirect *op_table[LSI_NUM_OPERATIONS]; /* vtable information */
} LsiRedirectProfile;

/**
 * Construct a new LsiRedirectProfile
 *
 * @param name Profile name
 * @returns A newly allocated LsiRedirectProfile
 */
LsiRedirectProfile *lsi_redirect_profile_new(const char *name);

/**
 * Free a previously allocated LsiRedirectProfile
 *
 * @param profile Pointer to an allocated LsiRedirectProfile
 */
void lsi_redirect_profile_free(LsiRedirectProfile *profile);

/**
 * Attempt insert of a redirect rule into this profile
 *
 * @param profile Pointer to a valid LsiRedirectProfile
 * @param redirect Pointer to an allocated LsiRedirect
 */
void lsi_redirect_profile_insert_rule(LsiRedirectProfile *self, LsiRedirect *redirect);

/* LsiRedirect APIs follow */

/**
 * Construct a new LsiRedirect to perform path replacements
 */
LsiRedirect *lsi_redirect_new_path_replacement(const char *source_path, const char *target_path);

/**
 * Attempt to free this redirect
 *
 * @param redirect Pointer to allocated LsiRedirect
 */
void lsi_redirect_free(LsiRedirect *self);

/*
 * Editor modelines  -  https://www.wireshark.org/tools/modelines.html
 *
 * Local variables:
 * c-basic-offset: 8
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 *
 * vi: set shiftwidth=8 tabstop=8 expandtab:
 * :indentSize=8:tabSize=8:noTabs=true:
 */
