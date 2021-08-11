# 1. 프로젝트 소개
#### Windows API를 이용한 폴더 모니터링 클라이언트
- 기술 스택 (C++, Windows API, REST, WebSocket, S3, DBMS 등)
- 사용된 라이브러리 (cpprestsdk, aws-sdk-cpp, sqlite3 등)
# 2. 프로젝트 목표
#### 클라이언트에서 모니터링 중인 폴더의 변경된 내역이나 다른 클라이언트에서 변경된 내역을 서버로 전송하여 클라이언트 간 특정 폴더를 실시간 공유
- 폴더의 하위 목록에 대한 메시지 모니터링
- 클라이언트 오프라인 상태에서의 폴더의 변경사항 파악
- 다른 클라이언트와 모니터링 중인 폴더 동기화
- 폴더의 변경사항 데이터를 서버로 전달
- Simple Storage Service 파일 업/다운로드
# 3. 클래스 설명
#### FolderWatcher
- 특정 폴더에 대한 Window 메시지 실시간 모니터링
#### WindowEventPusher
- Window 메시지 해석 및 이벤트 생성
#### InitialDiffEventPusher
- 클라이언트 오프라인 상태에서의 폴더의 변경사항에 대한 이벤트 생성
#### BroadcastEventPusher
- 서버로부터 전달받은 JSON 해석 및 이벤트 생성
# 4. 프로그램 요구사항 및 실행 방법
- sample
#### 
# 5. 프로젝트 구성도
- sample
