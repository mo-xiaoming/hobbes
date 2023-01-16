#include <hobbes/storage.H>

DEFINE_STORAGE_GROUP(WeatherMonitor, /* an arbitrary name for our data set */
                     3000, /* our maximum expected throughput in system pages (if we record up to
                              this limit, we either drop or block) */
                     hobbes::storage::Unreliable, /* we'd rather drop than block */
                     hobbes::storage::AutoCommit /* we don't need to correlate multiple records in a
                                                    batch (non-transactional) */
);

int main() {
  int i = 42; // try `unsigned long long` later
  while (true) {
    ++i;
    // HSTORE(WeatherMonitor, greetings, i);
    ::hobbes ::storage ::write(
        &WeatherMonitor, // StorageGroup address
        ::hobbes ::storage ::StorageStatement<
            decltype(WeatherMonitor),                               // StorageGroup type
            &WeatherMonitor,                                        // StorageGroup address
            PRIV_HPPF_TSTR("/home/mx/repos/hobbes/bin/producer.C"), // __FILE__
            15,                                                     // __LINE__
            PRIV_HPPF_TSTR("greetings"),                            // StorageStatement
            0,                                                      // flags
            PRIV_HPPF_TSTR(""),                                     // fmt string
            decltype(::hobbes ::storage ::makePayloadTypes(i))      // hstore_payload_types
            >::id,                                                  // StorageStatement::id
        i                                                           // arg(s)
    );
  }
}
