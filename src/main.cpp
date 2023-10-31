#include <templatebot/templatebot.h>

using json = nlohmann::json;
constexpr uint64_t DISBOARD_ID = 302050872383242240;

int main() {
    json configdocument;
    std::ifstream configfile("../config.json");
    configfile >> configdocument;

    dpp::cluster bot(configdocument["BOT_TOKEN"], dpp::i_message_content | dpp::i_default_intents);
    const dpp::snowflake& role_id = configdocument["BUMPER_ROLE_ID"];

    bot.on_log(dpp::utility::cout_logger());

    bot.on_message_create([&bot, role_id](const dpp::message_create_t& event) -> void {
        const dpp::message& msg = event.msg;
        if (msg.author.id == DISBOARD_ID && msg.embeds.size() == 1) {
            if (msg.embeds[0].image->url == "https://disboard.org/images/bot-command-image-bump.png") {
                dpp::user user = msg.interaction.usr;
                bot.guild_member_add_role(msg.guild_id, user.id, role_id, [&bot](const dpp::confirmation_callback_t& callback) -> void {
                    if (callback.is_error()) {
                        bot.log(dpp::ll_error, callback.get_error().message);
                    }
                });
                bot.start_timer([&bot, msg, role_id, user](const dpp::timer& h) -> void {
                    bot.guild_member_remove_role(msg.guild_id, user.id, role_id, [&bot](const dpp::confirmation_callback_t& callback) -> void {
                        if (callback.is_error()) {
                            bot.log(dpp::ll_error, callback.get_error().message);
                        }
                    });
                    bot.stop_timer(h);
                }, 7200);
            }
        }
    });

    bot.start(dpp::st_wait);
    return 0;
}
