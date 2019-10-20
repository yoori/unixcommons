/* 
 * This file is part of the UnixCommons distribution (https://github.com/yoori/unixcommons).
 * UnixCommons contains help classes and functions for Unix Server application writing
 *
 * Copyright (c) 2012 Yuri Kuznecov <yuri.kuznecov@gmail.com>.
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include <oci.h>


const OraText LOGIN[] = "BS_ADDBALPHA_30NOV";
const OraText PASSWORD[] = "adserver";
const OraText DB[] = "//oraclept/addbpt.ocslab.com";
#if 1
const OraText REQ[] = "select sysdate from dual";
#else
const OraText REQ[] =
            "SELECT "
            "bc.behav_params_id, "
            "( case when bc.trigger_type = 'U' then ch.url_list_id "
              "else ch.keyword_list_id end), "
            "bc.minimum_visits, "
            "nvl(bc.time_from, 0), "
            "nvl(bc.time_to, 0), "
            "bc.trigger_type, "
            "ch.AdServer_Status, "
            "greatest(bc.version, ch.version), "
            "ch.channel_id, "
            "lower(ch.country_code), "
            "weight "
          "FROM BehavioralParameters bc LEFT JOIN v_Channel ch "
            "ON(ch.channel_id = bc.channel_id) "
          "WHERE "
            "ch.channel_type in ('B', 'D')";
#endif

static OCIEnv *p_env;
static OCIError *p_err;
static OCISvcCtx *p_svc;
static OCIStmt *p_sql;
//static OCIBind *p_bnd = (OCIBind *) 0;

#define ROWS 0x400
ub4 count;
typedef OCIDefine* OCIDefinePtr;
static OCIDefinePtr* p_dfn;
typedef char* CharPtr;
CharPtr* data;

int rc;
char errbuf[100];
int errcode;
#define OCI_CHECK(x) \
do \
{ \
  rc = x; \
  if (rc) \
  { \
    OCIErrorGet((dvoid *)p_err, (ub4) 1, (OraText *) 0, &errcode, (OraText *) errbuf, (ub4) sizeof(errbuf), OCI_HTYPE_ERROR); \
    printf("Error %i %i while '%s': %.*s\n", rc, errcode, #x, 512, errbuf); \
    exit(8); \
  } \
} \
while (0)

void columns()
{
  OCI_CHECK(OCIAttrGet(p_sql, OCI_HTYPE_STMT, &count, 0, OCI_ATTR_PARAM_COUNT, p_err));
  printf("%u columns\n", count);

  p_dfn = new OCIDefinePtr[count];
  data = new CharPtr[count];

  for (ub4 i = 0; i < count; ++i)
  {
    // get next column info
    OCIParam* param_handle = 0;
    text* param_name = 0;
    ub4 name_len = 0;
    ub2 oci_data_type = 0;
    ub4 size = 0;

    OCI_CHECK(OCIParamGet(p_sql, OCI_HTYPE_STMT, p_err, (void **)&param_handle, i + 1));

    OCI_CHECK(OCIAttrGet(param_handle, OCI_DTYPE_PARAM, &param_name, &name_len, OCI_ATTR_NAME, p_err));
    OCI_CHECK(OCIAttrGet(param_handle, OCI_DTYPE_PARAM, &oci_data_type, 0, OCI_ATTR_DATA_TYPE, p_err));
    OCI_CHECK(OCIAttrGet(param_handle, OCI_DTYPE_PARAM, &size, 0, OCI_ATTR_DATA_SIZE, p_err));

    OCI_CHECK((OCIHandleFree(param_handle, OCI_DTYPE_PARAM),0));

    printf("%i %.*s %i %i ", i, name_len, param_name, oci_data_type, size);

    unsigned long oci_type;

    switch (oci_data_type)
    {
    case SQLT_INT: // integer
    case SQLT_LNG: // long
    case SQLT_UIN: // unsigned int
    case SQLT_NUM: // numeric
    case SQLT_FLT: // float
    case SQLT_VNU: // numeric with length
    case SQLT_PDN: // packed decimal
      printf("Number\n");
      oci_type = SQLT_VNU;
      size = sizeof (OCINumber);
      break;
    case SQLT_DAT: // date
    case SQLT_ODT: // oci date
      printf("Date\n");
      oci_type = SQLT_ODT;
      size = sizeof(OCIDate);
      break;
    case SQLT_DATE:
    case SQLT_TIME:
    case SQLT_TIME_TZ:
      printf("Time\n");
      oci_type = SQLT_TIME;
      size = sizeof(OCIDateTime*);
      break;
    case SQLT_TIMESTAMP:
    case SQLT_TIMESTAMP_TZ:
    case SQLT_TIMESTAMP_LTZ:
      printf("TimeStamp\n");
      oci_type = SQLT_TIMESTAMP;
      size = sizeof(OCIDateTime*);
      break;
    case SQLT_CHR: // character string
    case SQLT_STR: // zero-terminated string
    case SQLT_VCS: // variable-character string
    case SQLT_AFC: // ansi fixed char
    case SQLT_AVC: // ansi var char
    case SQLT_VST: // oci string type
      printf("TimeStamp\n");
      oci_type = SQLT_STR;
      size = size + 1;
      break;
    case SQLT_BLOB:
      printf("Blob\n");
      oci_type = SQLT_BLOB;
      size = sizeof(OCILobLocator*);
      break;
    default:
      abort();
    }

    data[i] = new char[ROWS * size];

    /* Define the select list items */
    OCI_CHECK(OCIDefineByPos(p_sql, &p_dfn[i], p_err, i + 1, &data[i], size, oci_type, 0, 0, 0, OCI_DEFAULT));
  }
}

void
trace(const char* message)
{
  write(STDERR_FILENO, message, strlen(message));
}

void* mall(void* ctxp, size_t size)
{
  assert(!ctxp);
  return malloc(size);
}

void* reall(void* ctxp, void* ptr, size_t size)
{
  assert(!ctxp);
  return realloc(ptr, size);
}

void fre(void* ctxp, void* ptr)
{
  assert(!ctxp);
  return free(ptr);
}

int
main()
{
  trace("TRACE: starting\n");

#if 0
  /* Initialize OCI */
  OCI_CHECK(OCIInitialize(OCI_THREADED, 0, mall, reall, fre));

  /* Initialize environment */
  OCI_CHECK(OCIEnvInit(&p_env, OCI_DEFAULT, 0, (dvoid **)0));
#else
  OCI_CHECK(OCIEnvCreate(&p_env, OCI_THREADED | OCI_OBJECT, 0, mall, reall, fre, 0, (dvoid **)0));
#endif

  /* Initialize handles */
  OCI_CHECK(OCIHandleAlloc(p_env, (dvoid **)&p_err, OCI_HTYPE_ERROR, 0, (dvoid **)0));
  OCI_CHECK(OCIHandleAlloc(p_env, (dvoid **)&p_svc, OCI_HTYPE_SVCCTX, 0, (dvoid **)0));

  /* Connect to database server */
  OCI_CHECK(OCILogon(p_env, p_err, &p_svc, LOGIN, sizeof(LOGIN) - 1, PASSWORD, sizeof(PASSWORD) - 1, DB, sizeof(DB) - 1));

  trace("TRACE: preparing\n");

  /* Allocate and prepare SQL statement */

  OCI_CHECK(OCIHandleAlloc(p_env, (dvoid **)&p_sql, OCI_HTYPE_STMT, 0, (dvoid **)0));
  OCI_CHECK(OCIStmtPrepare(p_sql, p_err, REQ, sizeof(REQ) - 1, OCI_NTV_SYNTAX, OCI_DEFAULT));

#if 0
  /* Bind the values for the bind variables */
  p_bvi = 10; /* Use DEPTNO=10 */
  rc = OCIBindByName(p_sql, &p_bnd, p_err, (text *) ":x",
    -1, (dvoid *) &p_bvi, sizeof(int), SQLT_INT, (dvoid *) 0,
    (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
#endif

  trace("TRACE: executing\n");

  /* Execute the SQL statement */
  OCI_CHECK(OCIStmtExecute(p_svc, p_sql, p_err, 0, 0, (const OCISnapshot *)0, (OCISnapshot *)0, OCI_DEFAULT));

  trace("TRACE: binding\n");

  columns();

  trace("TRACE: fetching\n");

  int row = 0;
  do
  {
    /* Fetch the remaining data */
    rc = OCIStmtFetch(p_sql, p_err, ROWS, OCI_FETCH_NEXT, OCI_DEFAULT);

#if 1
    if (!(row & 0xFFFF))
    {
      printf("%i\n", row);
    }
    row += ROWS;
#else
    for (int i = 0; i < FIELDS; i++)
    {
      printf(" %s", p_sli[i]);
    }
    printf("\n");
#endif
  }
  while (rc != OCI_NO_DATA);

  trace("TRACE: deinitializing\n");

  for (ub4 i = 0; i < count; ++i)
  {
    delete [] data[i];
  }
  delete [] data;
  delete [] p_dfn;

  OCI_CHECK(OCILogoff(p_svc, p_err)); /* Disconnect */
  OCI_CHECK(OCIHandleFree(p_sql, OCI_HTYPE_STMT)); /* Free handles */
  OCI_CHECK((OCIHandleFree(p_svc, OCI_HTYPE_SVCCTX),0));
  OCI_CHECK(OCIHandleFree(p_err, OCI_HTYPE_ERROR));

  trace("TRACE: done\n");

  return 0;
}
