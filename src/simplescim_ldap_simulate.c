#include "simplescim_ldap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "simplescim_error_string.h"
#include "simplescim_arbval.h"
#include "simplescim_arbval_list.h"
#include "simplescim_user.h"
#include "simplescim_user_list.h"

struct attribute {
	const char *name;
	size_t n_values;
	const char *values[10];
};

struct user {
	size_t n_attributes;
	struct attribute attributes[10];
};

static struct user g_users[] = {
	{3, {
		{"uid", 1, {"1"}},
		{"fullName", 1, {"Test1 Testtest1"}},
		{"givenName", 1, {"Test1"}}
	}},
	{3, {
		{"uid", 1, {"2"}},
		{"fullName", 1, {"Test2 Testtest2"}},
		{"givenName", 1, {"Test2"}}
	}}
};

static struct simplescim_arbval_list *get_values(struct attribute *ap)
{
	struct simplescim_arbval_list *values;
	struct simplescim_arbval *value;
	size_t i;
	int err;

	values = simplescim_arbval_list_new(ap->n_values);

	for (i = 0; i < ap->n_values; ++i) {
		value = simplescim_arbval_string(ap->values[i]);

		if (value == NULL) {
			simplescim_arbval_list_delete(values);
			return NULL;
		}

		err = simplescim_arbval_list_append(
			values,
			value
		);

		if (err == -1) {
			simplescim_arbval_delete(value);
			simplescim_arbval_list_delete(values);
			return NULL;
		}
	}

	return values;
}

static struct simplescim_user *get_user(struct user *up)
{
	struct simplescim_user *user;
	char *attribute;
	struct simplescim_arbval_list *values;
	size_t i;
	int err;

	user = simplescim_user_new();

	for (i = 0; i < up->n_attributes; ++i) {
		attribute = strdup(up->attributes[i].name);

		if (attribute == NULL) {
			simplescim_error_string_set_errno(
				"get_users:"
				"strdup"
			);
			simplescim_user_delete(user);
			return NULL;
		}

		values = get_values(&up->attributes[i]);

		if (values == NULL) {
			free(attribute);
			simplescim_user_delete(user);
			return NULL;
		}

		err = simplescim_user_set_attribute(
			user,
			attribute,
			values
		);

		if (err == -1) {
			simplescim_arbval_list_delete(values);
			free(attribute);
			simplescim_user_delete(user);
			return NULL;
		}
	}

	return user;
}

struct simplescim_user_list *simplescim_ldap_get_users()
{
	struct simplescim_user_list *users;
	struct simplescim_user *user;
	struct simplescim_arbval *uid;
	size_t i;
	int err;

	users = simplescim_user_list_new();

	if (users == NULL) {
		return NULL;
	}

	for (i = 0; i < sizeof(g_users) / sizeof(struct user); ++i) {
		user = get_user(&g_users[i]);

		if (user == NULL) {
			simplescim_user_list_delete(users);
			return NULL;
		}

		uid = simplescim_user_get_uid(user);

		if (uid == NULL) {
			simplescim_user_delete(user);
			simplescim_user_list_delete(users);
			return NULL;
		}

		err = simplescim_user_list_insert_user(
			users,
			uid,
			user
		);

		if (err == -1) {
			simplescim_arbval_delete(uid);
			simplescim_user_delete(user);
			simplescim_user_list_delete(users);
			return NULL;
		}
	}

	return users;
}
