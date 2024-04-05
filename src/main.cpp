#include <templatebot/templatebot.h>

using json = nlohmann::json;
constexpr uint64_t DISBOARD_ID{302050872383242240};

// This will only return a value on Linux.
int64_t proc_self_value(const std::string_view find_token) {
	int64_t ret{0};
	std::ifstream self_status{"/proc/self/status"};
	while (self_status) {
		std::string token;
		self_status >> token;
		if (token == find_token) {
			self_status >> ret;
			break;
		}
	}
	self_status.close();
	return ret;
}

int64_t rss() {
	return proc_self_value("VmRSS:") * 1024;
}

int main() {
	json config_document;
	std::ifstream config_file{"../config.json"};
	config_file >> config_document;

	/* Create bot */
	dpp::cluster bot{config_document["BOT_TOKEN"], dpp::i_message_content | dpp::i_default_intents, 0, 0, 1, true, dpp::cache_policy::cpol_none};

	bot.on_log(dpp::utility::cout_logger());

	bot.on_slashcommand([&bot](const dpp::slashcommand_t& event) {
		if (event.command.get_command_name() == "set_role") {
			auto role{std::get<dpp::snowflake>(event.get_parameter("role"))};

			if (role.empty()) {
				event.reply(dpp::message{"You didn't specify a role!"}.set_flags(dpp::m_ephemeral));
				return;
			}

			json j;
			j["ROLE_ID"] = role.str();
			std::ofstream guild_config{"../" + event.command.guild_id.str() + ".json"};
			guild_config << j.dump() << "\n";
			guild_config.close();

			event.reply(dpp::message{"Updated the role!"}.set_flags(dpp::m_ephemeral));
		} else if (event.command.get_command_name() == "info") {
			std::ifstream guild_config{"../" + event.command.guild_id.str() + ".json"};
			dpp::snowflake role_id{};

			if (guild_config.good()) {
				json j{};
				guild_config >> j;
				role_id = j["ROLE_ID"];
			}

			dpp::embed embed = dpp::embed()
				.set_title("Bump Watcher - Information")
				.set_footer(dpp::embed_footer{
					.text = "Requested by " + event.command.usr.format_username(),
					.icon_url = event.command.usr.get_avatar_url()
				})
				.set_colour(dpp::colours::aqua)
				.add_field("Bot Uptime", bot.uptime().to_string(), true)
				.add_field("Memory Usage", std::to_string(rss() / 1024 / 1024) + "M", true)
				.add_field("Role ID:", role_id ? std::to_string(role_id) : "Not set!", true);

			event.reply(dpp::message().add_embed(embed));
		}
	});

	/* When a message has been sent */
	bot.on_message_create([&bot](const dpp::message_create_t& event) {
		/* Is the author the DISBOARD bot? */
		if (event.msg.author.id != DISBOARD_ID) {
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

		std::ifstream guild_config{"../" + event.msg.guild_id.str() + ".json"};

		if (guild_config.bad()) {
			event.reply("Could not give a role based off this message as the role has not been setup! Please use `/set_role`.");
			return;
		}

		json j;
		guild_config >> j;

		const dpp::message& msg = event.msg;
		const dpp::user& user = event.msg.interaction.usr;
		const dpp::snowflake role_id{j["ROLE_ID"]};

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
		if (dpp::run_once<struct setup_commands>()) {
			dpp::slashcommand setrole_command{"set_role", "Set the role that Bump Watcher should add to the the most recent bumper!", bot.me.id};
			setrole_command.add_option(
				dpp::command_option{dpp::co_role, "role", "The role that the bot should add for the most recent bumper!", true}
			);

			dpp::slashcommand info_command{"info", "Get information about the bot!", bot.me.id};

			bot.global_bulk_command_create({ setrole_command, info_command });
		}

		if (dpp::run_once<struct presence_timer>()) {
			bot.current_application_get([&bot](const dpp::confirmation_callback_t& callback) {
				auto app{callback.get<dpp::application>()};
				bot.set_presence(dpp::presence{dpp::ps_online, dpp::at_watching, "DISBOARD on " + std::to_string(app.approximate_guild_count) + " servers!"});
			});

			bot.start_timer([&bot](const dpp::timer& timer) {
				bot.current_application_get([&bot](const dpp::confirmation_callback_t& callback) {
					auto app{callback.get<dpp::application>()};
					bot.set_presence(dpp::presence{dpp::ps_online, dpp::at_watching, "DISBOARD on " + std::to_string(app.approximate_guild_count) + " servers!"});
				});
			}, 120);
		}
	});

	bot.start(dpp::st_wait);
	return 0;
}
