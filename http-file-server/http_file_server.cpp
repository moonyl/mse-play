#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

// Content-Type을 결정하는 함수
std::string get_content_type(const std::string &path) {
  size_t dot_pos = path.find_last_of('.');
  if (dot_pos == std::string::npos)
    return "text/plain";
  std::string ext = path.substr(dot_pos);
  std::map<std::string, std::string> types = {
      {".html", "text/html"},        {".htm", "text/html"},
      {".css", "text/css"},          {".js", "application/javascript"},
      {".json", "application/json"}, {".txt", "text/plain"},
      {".mp4", "video/mp4"},         {".jpg", "image/jpeg"},
      {".jpeg", "image/jpeg"},       {".png", "image/png"},
      {".gif", "image/gif"}};
  auto it = types.find(ext);
  return it != types.end() ? it->second : "text/plain";
}

int main(int argc, char *argv[]) {
  const int PORT = 3000;
  std::string BASE_DIR = "."; // 기본 디렉터리

  // 명령줄 인자로 디렉터리 경로 받기
  if (argc > 1) {
    BASE_DIR = argv[1];
  }

  // 소켓 생성
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1) {
    std::cerr << "소켓 생성 실패" << std::endl;
    return 1;
  }

  // 주소 설정
  sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  // 바인딩
  if (bind(server_fd, (sockaddr *)&address, sizeof(address)) == -1) {
    std::cerr << "바인딩 실패" << std::endl;
    close(server_fd);
    return 1;
  }

  // 리스닝
  if (listen(server_fd, 10) == -1) {
    std::cerr << "리스닝 실패" << std::endl;
    close(server_fd);
    return 1;
  }

  std::cout << "서버가 포트 " << PORT << "에서 실행 중..." << std::endl;

  while (true) {
    // 클라이언트 연결 수락
    int client_fd = accept(server_fd, nullptr, nullptr);
    if (client_fd == -1) {
      std::cerr << "연결 수락 실패" << std::endl;
      continue;
    }

    // 요청 읽기 (간단히 버퍼로)
    char buffer[1024] = {0};
    read(client_fd, buffer, 1024);
    std::string request(buffer);

    // GET 요청인지 확인
    if (request.find("GET") == 0) {
      std::string file_path;

      // 경로 파싱
      size_t start = request.find(" ");
      if (start != std::string::npos) {
        start += 1; // "GET " 이후
        size_t end = request.find(" ", start);
        if (end != std::string::npos) {
          std::string path = request.substr(start, end - start);
          if (path == "/")
            path = "/index.html"; // 루트 요청 시 index.html

          // /media/filename 경로
          if (path.find("/media/") == 0) {
            std::string filename = path.substr(7); // "/media/" 제거
            file_path = BASE_DIR + "/" + filename;
          } else {
            // ht_root에서 제공
            file_path = "../ht_root" + path;
          }
        } else {
          file_path = ""; // 잘못된 경로
        }
      } else {
        file_path = ""; // 잘못된 요청
      }

      // 파일 크기 확인
      struct stat file_stat;
      if (file_path.empty() || stat(file_path.c_str(), &file_stat) == -1 ||
          !S_ISREG(file_stat.st_mode)) {
        // 파일 없음 응답
        std::string response = "HTTP/1.1 404 Not Found\r\n\r\nFile not found";
        send(client_fd, response.c_str(), response.size(), 0);
      } else {
        // 파일 열기
        std::ifstream file(file_path, std::ios::binary);
        if (!file) {
          std::string response = "HTTP/1.1 404 Not Found\r\n\r\nFile not found";
          send(client_fd, response.c_str(), response.size(), 0);
        } else {
          // HTTP 헤더 전송
          std::string header = "HTTP/1.1 200 OK\r\n";
          header +=
              "Content-Length: " + std::to_string(file_stat.st_size) + "\r\n";
          header += "Content-Type: " + get_content_type(file_path) + "\r\n";
          header += "\r\n";
          send(client_fd, header.c_str(), header.size(), 0);

          // 청크 단위로 파일 전송 (64KB)
          const size_t CHUNK_SIZE = 65536;
          char file_buffer[CHUNK_SIZE];
          while (file.read(file_buffer, CHUNK_SIZE) || file.gcount() > 0) {
            send(client_fd, file_buffer, file.gcount(), 0);
          }
          file.close();
        }
      }
    } else {
      // 잘못된 요청
      std::string response = "HTTP/1.1 400 Bad Request\r\n\r\nBad Request";
      send(client_fd, response.c_str(), response.size(), 0);
    }

    // 연결 종료
    close(client_fd);
  }

  close(server_fd);
  return 0;
}