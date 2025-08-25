# fMP4 변환 가이드

이 문서는 표준 MP4 파일을 Media Source Extensions(MSE) 호환 fMP4(Fragmented MP4) 형식으로 변환하는 방법을 설명합니다. 웹 브라우저에서 MSE를 사용하여 비디오를 스트리밍 재생하려면 fMP4 형식이 필요합니다.

## 1. FFmpeg 설치

fMP4 변환을 위해 FFmpeg를 사용합니다. FFmpeg는 오픈소스 멀티미디어 프레임워크로, 다양한 포맷 변환을 지원합니다.

### 설치 방법
- **Windows**: 공식 사이트에서 바이너리 다운로드 (https://ffmpeg.org/download.html)
- **macOS**: Homebrew를 사용: `brew install ffmpeg`
- **Linux**: 패키지 매니저 사용 (예: Ubuntu: `sudo apt install ffmpeg`)

설치 확인:
```bash
ffmpeg -version
```

## 2. 기본 변환 명령어

표준 MP4를 fMP4로 변환하는 기본 명령어입니다.

```bash
ffmpeg -i input.mp4 -c:v copy -c:a copy -movflags frag_keyframe+empty_moov+default_base_moof -f mp4 output.mp4
```

### 옵션 설명
- `-i input.mp4`: 입력 파일 지정
- `-c:v copy`: 비디오 코덱 복사 (재인코딩 없이)
- `-c:a copy`: 오디오 코덱 복사
- `-movflags frag_keyframe+empty_moov+default_base_moof`: fMP4 생성 옵션
  - `frag_keyframe`: 키프레임 단위로 조각화
  - `empty_moov`: Movie Box를 빈 상태로 생성 (스트리밍 최적화)
  - `default_base_moof`: base-data-offset를 절대 주소로 설정 (MSE 호환)
- `-f mp4`: 출력 포맷 지정

## 3. 고급 옵션

특정 요구사항에 따라 추가 옵션을 사용할 수 있습니다.

### 코덱 재인코딩 (필요 시)
코덱이 MSE에서 지원되지 않는 경우 재인코딩:
```bash
ffmpeg -i input.mp4 -c:v libx264 -preset fast -c:a aac -movflags frag_keyframe+empty_moov+default_base_moof -f mp4 output.mp4
```
- `-c:v libx264`: H.264 코덱 사용
- `-preset fast`: 인코딩 속도/품질 균형

### 해상도/비트레이트 조정
```bash
ffmpeg -i input.mp4 -c:v libx264 -preset fast -vf scale=1280:720 -b:v 2000k -c:a aac -movflags frag_keyframe+empty_moov+default_base_moof -f mp4 output.mp4
```
- `-vf scale=1280:720`: 해상도 변경
- `-b:v 2000k`: 비디오 비트레이트 설정

## 4. 변환 예시

### 예시 1: 기본 변환
```bash
ffmpeg -i video.mp4 -c:v copy -c:a copy -movflags frag_keyframe+empty_moov+default_base_moof -f mp4 fmp4_video.mp4
```

### 예시 2: 재인코딩 포함
```bash
ffmpeg -i old_video.avi -c:v libx264 -preset fast -c:a aac -movflags frag_keyframe+empty_moov+default_base_moof -f mp4 fmp4_video.mp4
```

## 5. 변환 후 검증

변환된 파일이 fMP4인지 확인:
```bash
ffprobe fmp4_video.mp4
```

- 출력에서 `frag_keyframe` 관련 정보 확인
- MSE 호환 코덱 (H.264/AAC)인지 검증

웹 브라우저에서 테스트: `MediaSource.isTypeSupported('video/mp4; codecs="avc1.42E01E, mp4a.40.2"')`

## 6. 주의사항

- **코덱 호환성**: MSE는 H.264 비디오와 AAC 오디오를 주로 지원. 다른 코덱은 재인코딩 필요.
- **오류 해결**: TFHD base-data-offset 에러 시, 위 명령어의 `-movflags` 옵션 사용.
- **파일 크기**: fMP4는 스트리밍에 최적화되지만, 파일 크기가 약간 증가할 수 있음.
- **성능**: 대용량 파일 변환 시 시간이 걸릴 수 있음.

## 7. 추가 리소스

- FFmpeg 공식 문서: https://ffmpeg.org/documentation.html
- MSE 사양: https://www.w3.org/TR/media-source/
- ISO BMFF: https://www.w3.org/TR/mse-byte-stream-format-isobmff/