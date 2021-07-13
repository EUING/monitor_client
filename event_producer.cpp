#include "event_producer.h"

#include <stdint.h>

#include <vector>
#include <memory>

#include "event_queue.h"
#include "base_event.h"
#include "upload_event.h"
#include "download_event.h"
#include "conflict_event.h"
#include "remove_event.h"
#include "local_remove_event.h"
#include "rename_event.h"
#include "diff_check.h"
#include "common_struct.h"

namespace monitor_client {
	EventProducer::EventProducer(const std::shared_ptr<EventQueue>& event_queue) : event_queue_(event_queue) {
	}

	void EventProducer::PushEvent(std::unique_ptr<EventPusher> event_pusher) {
		event_pusher->PushEvent(event_queue_);
	}
}  // namespace monitor_client