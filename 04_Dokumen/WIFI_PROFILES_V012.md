# Wi-Fi tersimpan dan cadangan — v0.12.0

Historical v0.12 policy/UI: [WIFI_V013.md](WIFI_V013.md) replaces ordered automatic
attempts with strongest visible saved Wi-Fi first and a simpler always-visible
panel. The NVS schema and profile endpoint contracts below remain, but /order
only breaks signal ties; absent SSIDs now require explicit manual connect.

Permintaan pengguna19 Oktober2026 mengganti kebijakan SSID tunggal: robot boleh
menyimpan jaringan pilihan operator dan otomatis mencoba profil berikutnya saat
gagal. Aturan lama yang menulis ulang HuaweiJIN setiap boot bersifat historis;
HuaweiJIN tetap default instalasi pertama bila tidak ada profil legacy yang valid.
Password hotspot pemulihan tetap12345678, akses lab tetap terbuka.

## Cara menggunakan dashboard

1. Buka dashboard dari IP router robot, atau sambungkan PC/ponsel ke hotspot
   `ai-robot-…-setup` lalu buka `http://192.168.4.1/`.
   Bila setelah OTA masih terlihat tersambung tetapi halaman tidak merespons,
   putus/sambungkan ulang Wi-Fi PC ke SSID hotspot yang sama sebelum mencoba lagi.
2. Masuk **Pengaturan Wi-Fi & jaringan cadangan**, lepaskan kendali dan matikan
   ArduinoOTA bila aktif. Tekan **Scan jaringan sekitar**; tunggu hasilnya.
3. Pilih SSID dari hasil scan atau ketik manual untuk jaringan tersembunyi.
   Isi password8–63 karakter; centang jaringan terbuka hanya bila memang tanpa
   password. Scan tidak memilih atau menyimpan jaringan secara otomatis.
4. Tekan **Simpan profil (tidak langsung pindah)**. Maksimal5 SSID unik. SSID
   sama memperbarui password di posisi prioritasnya; password lama tidak pernah
   ditampilkan/diisi ulang. Password kosong berarti open, bukan mempertahankan lama.
5. Gunakan **Naik/Turun** untuk mengatur urutan. Klik **Sambung** bila ingin pindah
   sekarang. Menghapus profil yang sedang dipakai juga menjadwalkan perpindahan.
6. Setelah berpindah, IP router bisa berubah. Buka IP baru dari router/operator
   atau akses hotspot robot. Sesi kontrol lama tidak dilanjutkan; engine perlu
   koneksi/acquire dan target baru secara eksplisit.

Scan, penulisan profil dan permintaan pindah hanya diterima saat tidak ada owner,
tidak maintenance/OTA, dan operasi jaringan lain selesai. Dashboard hanya melepas
sesi miliknya sendiri; server menolak409 bila engine/orang lain masih memegang
kendali. Tidak ada global stop otomatis akibat klik pengaturan operator lain.

## Pemilihan dan failover

- Recovery AP aktif sejak boot dan dipertahankan, termasuk ketika STA tersambung.
  AP dan STA memakai radio yang sama; perpindahan channel/scan bisa mengganggu
  sementara akses AP. AP aktif bukan janji koneksi radio tanpa gangguan.
  Pengukuran perangkat menunjukkan request bisa timeout selama percobaan pindah;
  tunggu fase waiting/connected sebelum mengubah profil lagi.
- Boot mencoba prioritas1, kemudian2 dst. Setiap profil mendapat waktu15 detik
  untuk asosiasi dan DHCP. SSID tersembunyi ikut dicoba; tidak perlu terlihat scan.
- Semua gagal: berhenti mencoba30 detik, lalu ulangi dari prioritas1. Selama
  menunggu, AP tetap ada; retry berikutnya ditunda bila ada owner di AP.
- Saat koneksi berhasil, robot tetap di jaringan tersebut. Tidak roaming hanya
  karena ada jaringan lebih kuat/prioritas lebih tinggi; tidak ada scan latar
  belakang berkala ketika robot dikendalikan.
- Saat link yang sebelumnya mendapat IP hilang, kendali dibatalkan dan fase
  failover mengunci acquire/command. Coba daftar mulai prioritas1. Permintaan
  Sambung manual mencoba SSID itu dahulu, lalu profil lain menurut prioritas.
- Selama pengaturan/pergantian/scan, `network_busy` mengunci kontrol baru. Roda0,
  laserOFF, servo menahan referensi sebelumnya; tidak ada aktivasi pulsa baru.
  Setelah fase selesai, tidak ada target/session yang otomatis dihidupkan kembali.
- Arduino auto-reconnect dimatikan agar tidak berlomba dengan rotasi profil.
  Firmware500 ms expiry tetap aktif di task aktuator; loop jaringan tidak
  menjamin penanganan event loss seketika jika main HTTP task sedang terlambat.
- Terhubung berarti mendapat koneksi Wi-Fi/IP, bukan tes internet. Captive portal
  atau server backend yang mati tidak memicu pindah jaringan. Tidak ada jaminan
  fitur ini memperbaiki request TCP yang melewati deadline200 ms.

Gunakan Wi-Fi2.4 GHz dengan password Personal atau open. WPA Enterprise,
portal login, PSK hex64 karakter dan konfigurasi sertifikat tidak didukung UI ini.
SSID1–32 byte UTF-8, tanpa karakter kontrol; password kosong atau ASCII8–63.

## Penyimpanan dan migrasi

Satu key NVS baru `robot/wifiProfilesV1` menyimpan array berurutan SSID/password
sebagai satu write atomik NVS, tanpa erase total. Pada migrasi pertama, import
legacy `ssid/wifiPassword` bila valid; bila tidak, gunakan default lab. Setelah key
baru ada, daftar itu yang otoritatif, termasuk `[]` bila semua profil dihapus.
Legacy keys dan NVS lain dipertahankan; key auth lama apiToken/otaPassword tetap
dihapus sesuai kebijakan terdahulu. AP password dimigrasi ke default yang sama.
SDK Wi-Fi memakai storage RAM agar pergantian profil tidak menulis ulang flash
setiap percobaan; daftar Preferences di atas tetap persisten secara eksplisit.

Data profil baru tidak ditulis ulang setiap boot. Save yang gagal tidak mengubah
daftar RAM. JSON NVS rusak dibiarkan utuh dan memakai AP saja dengan
`storage_ready:false`; laporkan untuk pemulihan NVS terarah, jangan clear seluruh
NVS. Password disimpan lokal di NVS sesuai model perangkat yang ada, bukan
penyimpanan terenkripsi baru. API/status/serial tidak mengembalikan password
profil; source publik tidak menyertakan jaringan/password baru milik operator.

## HTTP API tambahan (port80)

Semua POST `Content-Type: application/json`, maksimum1024 byte, exact fields,
tidak menerima raw/escaped NUL. Tidak ada route Wi-Fi baru di TCP9000/HTTP81.

| Method / path | Body | Hasil |
|---|---|---|
| GET `/api/v1/wifi` | tidak ada | state, current_ssid, target_ssid, sta_ip, AP, storage_ready, profiles, scan_state, networks |
| POST `/api/v1/wifi/scan` | `{}` |202; scan async, GET untuk progres |
| POST `/api/v1/wifi/save` | `{"ssid":"nama","password":"password-baru"}` |200; save/update, tidak reconnect |
| POST `/api/v1/wifi/delete` | `{"ssid":"nama"}` |200 atau202 bila perlu pindah |
| POST `/api/v1/wifi/order` | `{"ssids":["utama","cadangan"]}` |200; setiap SSID tersimpan tepat sekali |
| POST `/api/v1/wifi/connect` | `{"ssid":"nama"}` |202; pindah setelah jeda400 ms untuk memberi waktu ACK HTTP terkirim |

400 invalid field/range/SSID/capacity;409 owner/network/OTA busy;503 storage/scan
atau acknowledgement task gagal. ACK202 berarti dijadwalkan, bukan sudah mendapat
IP. Password tidak muncul pada GET: `profiles:[{ssid,priority,secured}]`.
`networks:[{ssid,rssi,secured}]` maksimal24 SSID terdeteksi, duplikat digabung
menjadi RSSI terkuat. Scan request15 detik deadline; hasil tidak dijamin tetap
mutakhir. Scan tidak berjalan sambil ada sesi kendali aktif.

Legacy POST `/api/v1/settings` dengan `{ssid,password}` kini save+jadwalkan
connect, bukan hanya menerimaHuaweiJIN. Scan, save dan reconnect tunduk guard yang
sama. Serial `wifi <ssid>|<password>` juga save+connect saat idle dan bukan OTA.

HTTP80/81 status menambah `network_busy`, `wifi_state`, dan `sta_target_ssid`
menjadi profil yang sedang/terakhir dicoba. TCP status menambah `network_busy`.
Reason baru `wifi_change`, `wifi_disconnected`, `wifi_ready`; counter event Wi-Fi
tetap mencakup disconnect yang disengaja saat profil diputar, bukan jumlah outage
RF independen. `wifi_ready` berarti guard jaringan dilepas, belum tentu STA sukses;
lihat wifi_state/sta_connected. Kontrak command TCP tidak berubah.

## Verifikasi

Baca entry v0.12.0 di VERIFICATION untuk build/perangkat terbaru. Host tests
menjalankan network_service.cpp sebenarnya dengan fake Wi-Fi/NVS dan pure control
logic. Browser memakai HTTP tiruan. Ini menguji logika/API dan rendering, bukan
asosiasi ke beberapa router fisik. Belum ada password cadangan pengguna yang
diberikan, tidak ada jaringan/router yang diubah untuk pengujian. OTA tidak
dianggap dilakukan hanya karena source dipush; status upload dilaporkan terpisah.
