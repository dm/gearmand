AC_DEFUN([AX_LIB_MONGOCLIENT],
[
    AC_ARG_WITH([mongoclient],
        AS_HELP_STRING(
            [--with-mongoclient=@<:@ARG@:>@],
            [use MongoDB library @<:@default=yes@:>@, optionally specify the prefix for mongoclient library]
        ),
        [
        if test "$withval" = "no"; then
            WANT_MONGOCLIENT="no"
        elif test "$withval" = "yes"; then
            WANT_MONGOCLIENT="yes"
            ac_mongoclient_path=""
        else
            WANT_MONGOCLIENT="yes"
            ac_mongoclient_path="$withval"
        fi
        ],
        [WANT_MONGOCLIENT="yes"]
    )

    MONGOCLIENT_CFLAGS=""
    MONGOCLIENT_LDFLAGS=""
    MONGOCLIENT_VERSION=""

    if test "x$WANT_MONGOCLIENT" = "xyes"; then

        ac_mongoclient_header="mongo/client/dbclient.h"

        mongoclient_version_req=ifelse([$1], [], [2.0.0], [$1])
        mongoclient_version_req_shorten=`expr $mongoclient_version_req : '\([[0-9]]*\.[[0-9]]*\)'`
        mongoclient_version_req_major=`expr $mongoclient_version_req : '\([[0-9]]*\)'`
        mongoclient_version_req_minor=`expr $mongoclient_version_req : '[[0-9]]*\.\([[0-9]]*\)'`
        mongoclient_version_req_micro=`expr $mongoclient_version_req : '[[0-9]]*\.[[0-9]]*\.\([[0-9]]*\)'`
        if test "x$mongoclient_version_req_micro" = "x" ; then
            mongoclient_version_req_micro="0"
        fi

        mongoclient_version_req_number=`expr $mongoclient_version_req_major \* 1000000 \
                                   \+ $mongoclient_version_req_minor \* 1000 \
                                   \+ $mongoclient_version_req_micro`
                                   
                                   
                                   
        MONGO_VERSION=`mongo --version | cut -c 24-`                   
        mongo_version_major=`expr $MONGO_VERSION : '\([[0-9]]*\)'`
        mongo_version_minor=`expr $MONGO_VERSION : '[[0-9]]*\.\([[0-9]]*\)'`
        mongo_version_micro=`expr $MONGO_VERSION : '[[0-9]]*\.[[0-9]]*\.\([[0-9]]*\)'`
        if test "xmongo_version_micro" = "x"; then
            mysql_version_micro="0"
        fi

        mongo_version_number=`expr $mongo_version_major \* 1000000 \
                                   \+ $mongo_version_minor \* 1000 \
                                   \+ $mongo_version_micro`

        AC_MSG_CHECKING([for MongoDB library >= $mongoclient_version_req])
        mongo_version_check=`expr $mongo_version_number \>\= $mongoclient_version_req_number`
  
        if test "$mongo_version_check" = "1"; then
            AC_MSG_RESULT([yes])
            success="yes"
        else
            AC_MSG_RESULT([not found])
            success="no"
            WANT_MONGOCLIENT="no"
        fi                            
                                   
        if test "$ac_mongoclient_path" != ""; then
            ac_mongoclient_ldflags="-L$ac_mongoclient_path/lib"
            ac_mongoclient_cppflags="-I$ac_mongoclient_path/include"
        else
            for ac_mongoclient_path_tmp in /usr /usr/local /opt ; do
                if test -f "$ac_mongoclient_path_tmp/include/$ac_mongoclient_header" \
                    && test -r "$ac_mongoclient_path_tmp/include/$ac_mongoclient_header"; then
                    ac_mongoclient_path=$ac_mongoclient_path_tmp
                    ac_mongoclient_cppflags="-I$ac_mongoclient_path_tmp/include"
                    ac_mongoclient_ldflags="-L$ac_mongoclient_path_tmp/lib"
                    break;
                fi
            done
        fi
        
        ac_mongoclient_ldflags="$ac_mongoclient_ldflags -lmongoclient -lboost_thread -lboost_filesystem"
        

        if test "$success" = "yes"; then

            MONGOCLIENT_CFLAGS="$ac_mongoclient_cppflags"
            MONGOCLIENT_LDFLAGS="$ac_mongoclient_ldflags"
            MONGOCLIENT_VERSION=$mongo_version_number

            AC_SUBST(MONGOCLIENT_CFLAGS)
            AC_SUBST(MONGOCLIENT_LDFLAGS)
            AC_SUBST(MONGOCLIENT_VERSION)
            AC_DEFINE([HAVE_MONGOCLIENT], [], [Have the MONGOCLIENT library])
        fi
    fi
])
