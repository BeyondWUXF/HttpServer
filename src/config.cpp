//
// Created by wuxiaofeng on 2019/5/24.
//

#include "config.h"

config* config::ins_ = nullptr;

config::config(boost::asio::io_context &io, int argc, char *argv[]) : log_sigs_(io) {
    root_ = boost::filesystem::absolute(argv[0]).lexically_normal().parent_path().parent_path();
    // 环境变量及配置参数
    init_options(argc, argv);
    // 初始化日志输出
    init_logging(io);
    ins_ = this;
}

void config::init_options(int argc, char *argv[]) {
    boost::program_options::variables_map vm;
    boost::program_options::options_description cdesc("选项");
    cdesc.add_options()
            ("help", "[ ] 此帮助信息")
            ("environ", boost::program_options::value<std::string>(&environ_)->default_value("debug"), "[*] 当前运行环境表示 eg. debug/product")
            ("utils.worker", boost::program_options::value<int>(&worker)->default_value(4), "[ ] 工作线程数")
            ("utils.max_listen", boost::program_options::value<int>(&listen)->default_value(256), "[ ] 同时监听数")
            ("utils.log", boost::program_options::value<std::string>(&log_path_)->default_value(""), "[ ] 日志输出文件")
            ("utils.log_level", boost::program_options::value<boost::log::trivial::severity_level>(&log_level_)->default_value(boost::log::trivial::info), "[ ] log 等级: trace/debug/info/warning")
            ("utils.limit", boost::program_options::value<bool>(&limit)->default_value(false), "[ ] 是否开启限流")
            ("pika.host", boost::program_options::value<std::string>(&pika.host), "[*] pika地址.")
            ("pika.port", boost::program_options::value<int>(&pika.port), "[*] pika端口.")
            ("server.port", boost::program_options::value<int>(&port)->default_value(7001), "[ ] 监听端口");

    boost::program_options::store(
            parse_environment(cdesc, [](const std::string& var) {
                return var == "ENVIRON" ? "environ" : "";
            }), vm);
    boost::program_options::store(
            boost::program_options::parse_command_line(argc, argv, cdesc), vm);
    if(vm.count("help") > 0) {
        std::cout << cdesc << std::endl;
        exit(0);
    }
    // 配置文件路径按不同环境区分加载
    boost::filesystem::path cfile = root_.native() + std::string("/etc/HttpServer.") + vm["environ"].as<std::string>() + std::string(".ini");
    boost::program_options::store(
            boost::program_options::parse_config_file<char>(cfile.c_str(), cdesc), vm);
    boost::program_options::notify(vm);

    local_ip = local_addr();
}

void config::init_logging(boost::asio::io_context &io) {
    if (log_path_.length() == 0) {
        return;
    }

    boost::log::add_common_attributes();
    log_back_ = boost::make_shared<LogBackT>(boost::log::keywords::file_name = log_path_,
            boost::log::keywords::open_mode = std::ios::app,
            boost::log::keywords::auto_flush = true,
            boost::log::keywords::start_thread = true);
    auto log_sink = boost::make_shared<LogSinkT>(log_back_);

    log_sink->set_formatter (
            boost::log::expressions::stream
                    << "["
                    << boost::log::expressions::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
                    << "] (" << boost::log::trivial::severity
                    << ") " << boost::log::expressions::smessage
//		boost::log::expressions::format("[%1%]<%2%>(%3) %4%")
//		% boost::log::expressions::format_date_time< boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
//		% boost::log::expressions::attr<boost::log::attributes::current_thread_id::value_type>("ThreadID")
//		% boost::log::trivial::severity
//		% boost::log::expressions::smessage
    );
    boost::log::core::get()->add_sink(log_sink);
    boost::log::core::get()->set_filter(
            boost::log::trivial::severity >= log_level_
    );

    if (environ_ == "trace") {
        auto console_sink=boost::log::add_console_log(std::clog, boost::log::keywords::format = "[%TimeStamp%] %Message%");
        boost::log::core::get()->add_sink(console_sink);
    }
    BOOST_LOG_TRIVIAL(info) << "HttpServer start..." << std::endl;
    // 信号处理进行日志重载
    log_sigs_.add(SIGUSR2);
    log_sigs_.async_wait(std::bind(&config::rotate_log, this, std::placeholders::_1, std::placeholders::_2));
}

void config::rotate_log(const boost::system::error_code &error, int signal_numer) {
    if (error) {
        BOOST_LOG_TRIVIAL(error) << "Signal error:" << error.message();
    } else {
        std::clog << "重载日志文件." << std::endl;
        BOOST_LOG_TRIVIAL(error) << "重载日志文件." << std::endl;  // 使用error级别，否则当日志级别设置为error时不会打印此信息
        boost::log::core::get()->flush();
        log_back_->rotate_file();
        BOOST_LOG_TRIVIAL(error) << "打开新日志文件." << std::endl;  // 使用error级别，否则当日志级别设置为error时不会打印此信息
        std::clog << "打开新日志文件." << std::endl;
        log_sigs_.async_wait(std::bind(&config::rotate_log, this, std::placeholders::_1, std::placeholders::_2));
    }
}
