# Analisis timeout kontrol — 19 Oktober2026 WIB

## Kesimpulan yang didukung bukti

Ada dua lapisan masalah yang perlu dipisahkan: keterlambatan request melewati
deadline client200 ms dan event putus/reconnect Wi-Fi yang tercatat pada robot.
Belum terbukti event Wi-Fi tertentu menyebabkan timeout log pukul16:39:30.549.
Tidak ada perubahan firmware, OTA, konfigurasi jaringan, atau perintah aktuator
yang dilakukan dalam analisis ini.

Log pengguna berasal dari PC teman yang menjalankan engine ML/game dan menuju
robot192.168.1.22. Tulisan `likely cause` tidak ditemukan dalam client resmi pada
checkout lokal; source aplikasi/log lengkap PC tersebut belum diperiksa.

## Fakta

- Log melaporkan timeout pada `status`, last good reply201 ms, target age14 ms,
  commands ACK15/sent15. Ini menunjukkan15 command sebelumnya diakui; tidak
  membuktikan command berikutnya diproses atau Wi-Fi radio terputus.
- Robot pada alamat baru192.168.1.22 cocok MACe0:72:a1:fd:0d:88, firmwarev0.11.0,
  buildUTC2026-10-19T03:04:54Z. Alamat lama192.168.1.66 tidak merespons saat probe;
  kegagalan alamat lama bukan bukti kondisi robot pada alamat barunya.
- Snapshot sekitar16:49 WIB: uptime9,328,203 ms (~2h35m), ownernone/session0,
  reasonreleased, wheels0, laserfalse, servo pulsesenabled/reference110.5/110.5.
  RSSI−58 dBm, freeheap206004/min152856. Tidak ada pengukuran sudut fisik.
- Counter sejak boot: wifi_disconnect_count27, wifi_connect_count2,
  last reason36. Header SDK pinned memetakan36 ke WIFI_REASON_STA_LEAVING.
  Firmware juga memanggil WiFi.disconnect() pada retry koneksi setelah30 detik
  disconnected atau konfigurasi eksplisit. Jadi27 event tidak sama dengan27
  outage independen yang tidak disengaja atau bukti27 gangguan RF.
- Sampel HTTP81 GET/status16:50:10–16:50:30 WIB:40 request,0 gagal pada deadline1 s,
  1 request >=200 ms, worst234 ms. Uptime naik, counter tetap27/connect2, tidak
  ada reboot/disconnect baru teramati pada interval tersebut. Ini HTTP dari
  laptop pemeriksa, bukan latency TCP9000 dari PC engine.
- Laptop pemeriksa mengakses subnet robot lewat Ethernet192.168.1.122. Wi-Fi
  laptop sedang di AP Compact QC/subnet192.168.4.x, bukan SSID robot. Tidak ada
  jaringan/router/route/firewall yang diubah. Koneksi TCP9000 tidak dibuka supaya
  tidak berbenturan dengan single-client slot engine.

## Mekanisme yang direproduksi tanpa perangkat

Client resmi RobotTcpClient.request memakai satu deadline200 ms untuk request,
dan saat TimeoutError memanggil close(): socket ditutup dan session lokal0.
Hal itu berlaku juga untuk status(), bukan hanya command(). Firmware mengikat
sesi ke socket; EOF menyebabkan scoped release, roda0/laserOFF/referensi hold.
Dengan demikian telemetry yang terlambat dapat membatalkan kendali seluruh sesi.

Loopback terkontrol: server lokal merespons hello lalu menunda status230 ms.
Client mendapat TimeoutError setelah203 ms, menutup socket, server mengamati EOF.
Request hanya hello/status, command0, tanpa Wi-Fi/robot/aktuator. Reproduksi ini
membuktikan mekanisme pada SDK resmi, bukan akar delay pada engine pengguna.

Artefak lokal ignored:build/status-timeout-analysis.json,
build/robot22-readonly-status.json. Source acuan:robot_tcp_client.py,
tcp_peer.h, tcp_service.cpp, network_service.cpp.

## Inferensi dan yang belum diketahui

- Sangat mungkin sebagian label DISCONNECTED berasal dari keputusan client
  menutup socket setelah deadline, bukan server lebih dahulu menutup koneksi.
  Source aplikasi teman perlu diperiksa untuk memastikan jalurnya sama.
- Sampel234 ms menunjukkan deadline200 ms dapat terlewati walau endpoint masih
  merespons dan counter disconnect tidak berubah. Ini tidak membuktikan besaran
  latency TCP engine pada16:39.
- Ada histori event Wi-Fi, tetapi tidak tersedia timestamp/alasan per-event,
  kaitan retry lokal, disconnect duration, atau packet capture kejadian tersebut.
- Target age14 ms hanya menunjukkan timestamp yang dicatat aplikasi; belum
  membuktikan semantik freshness/antrean model benar.
- Belum diketahui kondisi PC engine (Ethernet/Wi-Fi, beban model/game/render,
  thread socket, concurrent status, interval/gap command, Nagle, antrean kirim),
  jumlah byte status sebelum timeout, atau delay parser versus perjalanan jaringan.
- Belum ada bukti menyalahkan power, RF, router atau reset sebagai akar spesifik.

## Arah perbaikan yang disarankan

1. Periksa source client engine. Log harus membedakan connect_timeout,
   reply_timeout, peer_eof/reset, protocol_reject, producer_expired dan
   lease_expired; tulis client_closed_after_timeout bila client yang menutup.
   Jangan otomatis menyimpulkan Wi-Fi dropout dari TimeoutError.
2. Pisahkan telemetry dari jalur command: satu socket TCP kontrol, request ACK
   serial/single-flight; status diagnostik lewat HTTP81 dengan rate rendah dan
   hanya satu status pending. Kegagalan telemetry menjadi telemetry_stale, tidak
   otomatis melepas socket kontrol jika command ACK/producer masih sehat.
   HTTP telemetry tetap menambah trafik dan perlu diukur; jangan membuka socket
   TCP kedua karena server hanya menerima satu client.
3. Pertahankan penghentian saat command gagal/producer stale dan firmware500 ms.
   Jangan menelan timeout pada stream yang sama lalu mengirim command berikutnya:
   balasan lama bisa muncul belakangan dan merusak korelasi ID. Jangan menambahkan
   automatic reconnect/reacquire/replay yang menghidupkan target lama.
4. Tuning deadline hanya setelah ukur worst-case/gap di PC engine. Memperbesar
   semua timeout menjadi1–5 detik tidak cocok dengan lease500 ms, dan tidak
   memperbaiki radio/reconnect. SDK saat ini membatasi I/O<=250 ms; perubahan
   semantik perlu tes yang membuktikan command tetap gagal aman.
5. Tambahkan diagnostik firmware pada revisi berikut: event Wi-Fi bertimestamp
   uptime dengan alasan/sumber retry eksplisit, durasi offline, TCP close reason,
   RX-complete/dispatch/TX timing, deadline hits dan sequence terakhir. Gunakan
   buffer bounded, tanpa log di spinlock/output task. Jangan klaim counter saat
   ini cukup untuk menentukan akar masalah historis.

Validasi lanjutan: PC engine merekam request/ACK timeline, command gaps dan
producer age bersama snapshot HTTP; bandingkan dengan counter/uptime robot.
Uji local mock status lambat tetapi command sehat setelah telemetry dipisahkan;
uji command gagal tetap stop, target stale tidak replay, EOF tidak auto-resume,
dan status tidak menghabiskan deadline command. Uji perangkat awal read-only;
gerak/laser tetap menunggu pose awal dan izin operator. Soak lama tidak diulang.
