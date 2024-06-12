#pragma once

#include "pproto/commands/base.h"

namespace pproto {

typedef data::MessageError ErrorInfo;

namespace error {

//--- 10 Ошибки по работе с базой данных ---
DECL_ERROR_CODE(connect_to_database,          10, "2361b42b-7324-4230-8d00-a1c9348d7a58", u8"Ошибка подключения к базе данных")
DECL_ERROR_CODE(begin_transaction,            10, "d521450f-73f9-49a0-afe8-831dd66af353", u8"Ошибка старта транзакции")
DECL_ERROR_CODE(commit_transaction,           10, "5bc8301e-c1c5-4fa6-aeb8-c1b7b86afefe", u8"Ошибка завершения транзакции")
DECL_ERROR_CODE(rollback_transaction,         10, "ee3f3100-4643-4adf-8c6b-ca823e460d60", u8"Ошибка отмены транзакции")

DECL_ERROR_CODE(select_sql_statement,         10, "f99e821c-7f40-4c6e-aea2-b64ab5ec064c", u8"Ошибка выполнения 'select' sql-запроса")
DECL_ERROR_CODE(insert_sql_statement,         10, "141463a6-6cce-41c5-8a05-2473152ffe57", u8"Ошибка выполнения 'insert' sql-запроса")
DECL_ERROR_CODE(update_sql_statement,         10, "76861790-a004-4170-82e7-3ad608448645", u8"Ошибка выполнения 'update' sql-запроса")
DECL_ERROR_CODE(insert_or_update_sql,         10, "a1ba61bd-8c6d-464a-83e0-1db0d3f25246", u8"Ошибка выполнения 'insert or update' sql-запроса")
DECL_ERROR_CODE(delete_sql_statement,         10, "4ea9c633-0dec-4922-bdf7-3c5fa65f1075", u8"Ошибка выполнения 'delete' sql-запроса")

//--- 20 Ошибки общего плана ---
DECL_ERROR_CODE(admin_password,               20, "3a32b445-3930-4408-94ed-2280c797ecb9", u8"Неверный пароль администратора")
DECL_ERROR_CODE(user_login,                   20, "c87bb11b-69c8-4183-a6e3-fb9f30b18035", u8"Ошибка авторизации")
DECL_ERROR_CODE(user_logout,                  20, "ea79f28f-2363-4c17-8e99-c6fb122e6d86", u8"Ошибка выхода из системы")
DECL_ERROR_CODE(user_name_empty,              20, "b2495575-4782-4501-8831-8229822de2e8", u8"Имя пользователя не может быть пустым. Обратитесь к администратору")
DECL_ERROR_CODE(mono_auth,                    20, "6e498644-b9dd-49b9-83b7-71437f579023", u8"Авторизован другой пользователь")
DECL_ERROR_CODE(local_auth,                   20, "b8a1d20c-f0c8-479d-9634-b308f908817d", u8"Пользователь не может быть авторизован удаленно")
DECL_ERROR_CODE(already_auth,                 20, "ed4501ee-b143-4671-9a8d-56008379c6f0", u8"Пользователь уже авторизован в данный момент")

////--- 40 Ошибки по работе с алгоритмами ---
//DECL_ERROR_CODE(algo_is_null,                 40, "9f603b40-cb7d-48c2-b370-5a2406ad863b", "Algorithm ID is null")
//DECL_ERROR_CODE(algo_is_exist,                40, "70cf1761-df8e-4265-abab-51cefd192699", "Algorithm ID not unique")

//--- 50 Ошибки по работе с json ---
//DECL_ERROR_CODE(json_parse_error,             50, "fd665c40-c2e1-4697-b673-b15e6db9c846", "Json parse error")


} // namespace error
} // namespace pproto
