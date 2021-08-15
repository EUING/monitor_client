# 1. 프로젝트 소개
#### Windows API를 이용한 폴더 모니터링 클라이언트
- 기술 스택 (C++, Windows API, REST, WebSocket, S3, DBMS 등)
- 사용된 라이브러리 (cpprestsdk, aws-sdk-cpp, sqlite3 등)
# 2. 프로젝트 목표
#### 클라이언트에서 모니터링 중인 폴더의 이벤트를 서버로 전송하여 클라이언트 간 특정 폴더를 실시간 공유
- 폴더의 하위 목록에 대한 메시지 모니터링
- 클라이언트 오프라인 상태에서의 폴더의 변경사항 파악
- 다른 클라이언트와 모니터링 중인 폴더 동기화
- 폴더의 변경사항 데이터를 서버로 전달
- Simple Storage Service 파일 업/다운로드
# 3. 클래스 설명
#### FolderWatcher
- 특정 폴더에 대한 Window 메시지 실시간 모니터링
#### UploadEvent, RenameEvent, RemoveEvent, DownloadEvent, ConflictEvent
- Monitor 서버와 통신하여 처리해야 할 이벤트
#### LocalRenameEvent, LocalRemoveEvent
- Broadcast 수신 후 로컬 클라이언트에서 처리해야 할 이벤트
#### EventQueue
- 이벤트를 순차적으로 처리하기 위한 Queue
#### EventFilter
- 전달받은 데이터를 필터링하여 이벤트 생성 후 Queue에 삽입
#### WindowEventPusher
- Window 메시지 해석 후 EventFilter에 데이터를 전달
#### InitialDiffEventPusher
- 클라이언트 오프라인 상태에서의 폴더의 변경사항에 대한 데이터를 EventFilter에 전달
#### BroadcastEventPusher
- Broadcast 해석 후 EventFilter에 데이터를 전달
#### EventProducer
- EventFilter및 EventQueue를 EventPusher에 전달
#### ItemDaoSqlite
- Sqliite을 이용해서 로컬 데이터베이스에 접근
#### LocalDb
- ItemDao를 이용해서 로컬 데이터베이스를 관리
#### ItemHttp
- Monitor 서버에 HTTP 메시지 전송
#### ItemS3
- MinIO 서버에 공유할 파일 업/다운로드
#### ItemRequest
- 로컬 데이터베이스, MinIO서버, Monitor 서버와 접근 및 통신
#### EventConsumer
- Queue에 있는 이벤트를 지속적으로 처리
#### WebSocket
- 서버와 WebSocket 연결 및 Broadcast 수신
# 4. 프로그램 요구사항 및 실행 방법
1. 필요한 라이브러리
    - cpprestsdk
    - aws-sdk-cpp
    - sqlite_manager
    - yaml-cpp

2. 실행 방법
    - 모니터링할 폴더에 .ignore 폴더를 생성 후 config.yaml 파일 생성
    - 명령 프롬프트에서 모니터링할 폴더 경로를 전달하여 monitor_client.exe 실행
# 5. 프로젝트 구성도
![슬라이드1](https://user-images.githubusercontent.com/85142102/129477519-e86e0178-b8e3-46a7-9279-5ea6ec0621b1.JPG)
![슬라이드2](https://user-images.githubusercontent.com/85142102/129477520-63e6e14d-c8da-4f76-b508-9822955b62b4.JPG)
![슬라이드3](https://user-images.githubusercontent.com/85142102/129477521-c7b1a2de-4588-44c7-b19a-8007b6cd5676.JPG)
![슬라이드4](https://user-images.githubusercontent.com/85142102/129477523-1859acef-bd60-4559-8a30-97a54cef635b.JPG)
