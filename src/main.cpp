#include <templatebot/templatebot.h>

using json = nlohmann::json;
constexpr uint64_t DISBOARD_ID = 302050872383242240;

int main() {
	json config_document;
	std::ifstream config_file("../config.json");
	config_file >> config_document;

	/* Create bot */
	dpp::cluster bot(config_document["BOT_TOKEN"], dpp::i_message_content | dpp::i_default_intents);
	/* Get role id from config */
	const dpp::snowflake role_id{config_document["BUMPER_ROLE_ID"]};

	bot.on_log(dpp::utility::cout_logger());

	/* When a message has been sent */
	bot.on_message_create([&bot, role_id](const dpp::message_create_t& event) {
		/* Is the author the DISBOARD bot? */
		if(event.msg.author.id != DISBOARD_ID) {
			return;
		}

		/* Make sure the message has only 1 embed */
		if (event.msg.embeds.size() != 1) {
			return;
		}

		/* If the embed's image url isn't the disboard bump image */
		if (event.msg.embeds[0].image->url != "https://disboard.org/images/bot-command-image-bump.png") {
			return;
		}

		const dpp::message& msg = event.msg;
		const dpp::user& user = event.msg.interaction.usr;

		/* Add a role to the guild_member for bumping! */
		bot.guild_member_add_role(msg.guild_id, user.id, role_id, [&bot, msg, role_id, user](const dpp::confirmation_callback_t& callback) {
			if (callback.is_error()) {
				bot.log(dpp::ll_error, callback.get_error().message);
				return;
			}

			/* Now start a timer to remove the role as we know all went well. */
			bot.start_timer([&bot, msg, role_id, user](const dpp::timer& h) {
				bot.guild_member_remove_role(msg.guild_id, user.id, role_id, [&bot](const dpp::confirmation_callback_t& callback) {
					if (callback.is_error()) {
						bot.log(dpp::ll_error, callback.get_error().message);
					}
				});

				bot.stop_timer(h);
			}, 7200);
		});
	});

	bot.on_ready([&bot](const dpp::ready_t& event) {
		if(dpp::run_once<struct presence_timer>()) {
			bot.start_timer([&bot](const dpp::timer& timer) {
				bot.current_application_get([&bot](const dpp::confirmation_callback_t& callback) {
					auto app = callback.get<dpp::application>();
					bot.set_presence(dpp::presence(dpp::ps_online, dpp::at_watching, "DISBOARD on " + std::to_string(app.approximate_guild_count) + " servers!"));
				});
			}, 120);
		}
	});

	bot.start(dpp::st_wait);
	return 0;
}
