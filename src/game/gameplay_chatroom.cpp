#include "gameplay.h"
#include "channel.h"
#include "pc.h"

#include "../common/utils.h"
#include "../common/packet.h"

game_chatroom::game_chatroom(std::shared_ptr<gamedata> const& data, uint16 room_id, std::string const& room_name, std::string const& room_pwd)
	: game(data, room_id, room_name, room_pwd){}

void game_chatroom::send_pc_create(pc *pc) {
	Packet p;
	WTHEAD(&p, 0x48);
	WTIU08(&p, 0);
	WTI16(&p, -1);
	WTIU08(&p, (uint8)pc_list.size());
	pc->gamedata(&p, true);
	send(&p);
}

void game_chatroom::send_pc_join(pc *pc) {
	// query all pc
	for (auto &pl : pc_list) {
		{
			Packet p;
			WTHEAD(&p, 0x48);
			WTIU08(&p, 7);
			WTI16(&p, -1);
			WTIU08(&p, (uint8)pc_list.size());
			pl->gamedata(&p, true);
			pc->send(&p);
		}
	}

	// query this pc to all
	{
		Packet p;
		WTHEAD(&p, 0x48);
		WTIU08(&p, 1);
		WTI16(&p, -1);
		pc->gamedata(&p, true);
		send(&p);
	}
}

void game_chatroom::pc_req_gamedata(pc * pc)
{

}

void game_chatroom::pc_loadmap_success(pc * pc)
{
}

void game_chatroom::addmaster(pc * pc)
{
	game::addmaster(pc);
	channel->sys_game_action(this, gCreate);
	channel->sys_pc_action(pc, lbUpdate);
}

void game_chatroom::pc_req_leave_game(pc * pc)
{
	if (pc_remove(pc)) {
		pc->game = nullptr;
		pc->game_id = -1;
		pc->channel_->sys_pc_action(pc, lbUpdate);

		sys_send_pcleave(pc); // send to all users in the room

		if ((uint8)pc_list.size() == 0) {
			valid = false;
			channel->sys_game_action(this, gDestroy);
		}
		else {
			channel->sys_game_action(this, gUpdate);
		}

		channel->send_pc_leave_game(pc);
	}
}
