
configure_file(configure.h.in configure.h @ONLY)

add_subdirectory(struct)
add_subdirectory(types)

install_public_headers(
        libmemcached-1.0

        alloc.h
        allocators.h
        analyze.h
        auto.h
        basic_string.h
        behavior.h
        callback.h
        callbacks.h
        defaults.h
        delete.h
        deprecated_types.h
        dump.h
        encoding_key.h
        error.h
        exception.hpp
        exist.h
        fetch.h
        flush_buffers.h
        flush.h
        get.h
        hash.h
        limits.h
        memcached.h
        memcached.hpp
        options.h
        parse.h
        platform.h
        quit.h
        result.h
        return.h
        sasl.h
        server.h
        server_list.h
        stats.h
        storage.h
        strerror.h
        touch.h
        triggers.h
        types.h
        verbosity.h
        version.h
        visibility.h
        )
