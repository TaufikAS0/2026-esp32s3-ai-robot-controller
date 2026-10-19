# Wi-Fi sederhana dan redundansi — v0.13.0

Previous: v0.12.0, pemilihan menurut urutan simpan dan pengaturan di bawah OTA.
Keputusan pengguna 19 Oktober 2026: dahulukan sinyal terkuat dari jaringan yang
sudah disimpan, tampilkan Wi-Fi aktif/sinyal, sederhanakan untuk orang awam.
Dokumen ini mengganti aturan pemilihan dan tampilan pada WIFI_PROFILES_V012.md.

## Cara pakai

1. Buka dashboard robot. **Wi-Fi robot** langsung terlihat di bagian atas:
   nama jaringan aktif, IP, serta batang sinyal dan nilai dBm dari robot.
2. Tekan **Cari Wi-Fi**, pilih nama jaringan, isi password, lalu
   **Simpan & sambungkan**. Maksimal lima jaringan. Centang tanpa password hanya
   untuk jaringan terbuka. Password tersimpan tidak ditampilkan kembali.
3. Berikutnya, cukup **Sambung** pada jaringan tersimpan atau **Hapus** bila tidak
   diperlukan. Tidak perlu mengatur urutan Naik/Turun.
4. Jika sedang di hotspot robot, buka http://192.168.4.1/.
   Panel membedakan koneksi robot ke router dari akses browser melalui hotspot.
   Ketika robot belum terhubung ke router, sinyal router ditulis “—”; angka sinyal
   hotspot dari laptop tidak tersedia melalui API robot dan tidak direka.

## Pemilihan otomatis

- Saat boot, setelah link STA hilang, dan pada siklus pencarian berikutnya:
  scan asinkron dahulu; pilih **SSID tersimpan yang terdeteksi**, sinyal RSSI
  terkuat terlebih dahulu. SSID asing tidak pernah dipakai otomatis.
- Jika kandidat gagal memperoleh koneksi/IP dalam 15 detik, coba kandidat berikut
  dari snapshot scan yang sama. Setiap kandidat dicoba paling sekali per siklus.
  RSSI sama memakai urutan simpan sebagai pemecah seri.
- Semua kandidat gagal: tunggu 30 detik, scan dan susun ulang. Tidak ada kandidat
  atau scan gagal: jangan mencoba SSID yang tidak terlihat terus-menerus;
  tunggu **120 detik** sebelum scan lagi agar hotspot lebih mudah diakses.
- **Sambung** manual mencoba profil pilihan dahulu (termasuk SSID tersembunyi/
  tidak terdeteksi), lalu kandidat tersimpan lain dari sinyal terkuat. Jaringan
  tersembunyi tidak termasuk pemilihan otomatis berbasis scan; gunakan Sambung.
- Jika koneksi sudah baik, tetap di jaringan itu. Tidak ada scan/roaming hanya
  karena sinyal jaringan lain lebih kuat. “Terhubung” berarti Wi-Fi mendapat IP,
  bukan pemeriksaan internet/backend. Password salah atau DHCP gagal tetap bisa
  menggagalkan jaringan bersinyal kuat.
- Hotspot pemulihan tetap aktif. Scan/association dan AP berbagi radio; ini bukan
  redundansi tanpa jeda. Permintaan HTTP/socket bisa terputus dan IP bisa berubah.
  Engine harus reconnect/acquire dengan target baru secara eksplisit.

## API dan kompatibilitas

GET `/api/v1/wifi` menambah:

- `selection:"strongest_saved"`, `busy`, `retry_in_ms`, `scan_age_ms`;
- `rssi`: RSSI router saat benar-benar tersambung, atau null;
- `current_ssid`: kosong saat tidak tersambung, bukan SSID percobaan terakhir;
- tiap profil menambah `rssi` dari scan terakhir, atau null. Sinyal hasil scan
  dapat menua dan diberi label berbeda dari RSSI koneksi aktif pada dashboard.

Daftar scan diurutkan dari sinyal terkuat dan dideduplikasi per SSID. Respons
dibatasi 24 nama; pemilihan tetap mempertimbangkan seluruh lima profil tersimpan
meskipun hasil scan publik penuh. POST `/wifi/order` dipertahankan untuk klien
lama, tetapi kini hanya memengaruhi pemecah seri; dashboard tidak menampilkannya.
Form baru menggunakan POST `/settings` `{ssid,password}` yang sudah ada untuk
menyimpan sekaligus menjadwalkan koneksi. `/wifi/save` tetap menyimpan tanpa pindah.
UI melakukan GET berkala, bukan scan radio berkala: 5 detik saat normal,
1,5 detik saat sibuk, 8 detik setelah gagal. Tidak ada POST otomatis saat membuka.

Unchanged: pin, partisi dua slot 6 MiB, core 3.3.10, port HTTP80/81/TCP9000,
akses LAN terbuka, NVS `robot/wifiProfilesV1`, timeout perintah 500 ms. Scan,
penyimpanan dan koneksi tetap menolak owner/OTA; link loss membatalkan kendali,
roda nol/laser mati/servo membekukan referensi. Tidak ada target lama diteruskan.
Upgrade: app-only OTA mempertahankan semua profil yang tersimpan; tidak erase NVS.

Tests dan Limitations perangkat/build: lihat entri v0.13.0 di VERIFICATION.md.
RSSI terkuat bukan jaminan latensi terbaik atau bebas gangguan.

Referensi API SDK: [Espressif Arduino-ESP32 Wi-Fi](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/wifi.html).
