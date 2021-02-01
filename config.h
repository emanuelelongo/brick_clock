typedef struct {
  bool enabled;
  unsigned char hour;
  unsigned char minute;
  bool mon;
  bool tue;
  bool wed;
  bool thu;
  bool fri;
  bool sat;
  bool sun;
  char led[24][8];
  char ssid[33];
  char password[65];
  char owkey[33];
  char owcity[32];
  char owcountry[3];
} config;

extern config curConfig;

bool config_load();
bool config_save();
void config_erase();