#include "url.h"

url::url(const std::string& s) {
	clear();
    assign(s);
}

url::url() {
	clear();
}

url::~url() {
	clear();
}

void url::assign(const std::string& s) {
	this->clear();

	size_t begin = 0, end = 0;
	std::string temp;
	// scheme
	end = s.find("://");
	if (end != std::string::npos) {
		scheme_.assign(s, begin, end - begin);
		begin = end + 3;
	}

	//userinfo
	end = s.find_first_of("@", begin);
	if (end != std::string::npos) {
		userinfo_.assign(s, begin, end - begin);
		begin = end + 1;
	}

	// host+port
	end = s.find_first_of("/?#", begin);
	temp.assign(s, begin, end - begin);
	std::vector<std::string> hp;
	boost::split(hp, temp, boost::is_any_of(":"));
	if (hp.size() == 2) {
		host_ = hp[0];
		port_ = hp[1];
	} else {
		host_ = hp[0];
		default_port();
	}

	if (end == std::string::npos)
		return;
	// path
	if (s[end] == '/') {
		begin = end + 1;
		end = s.find_first_of("?#", begin);
		path_.assign(s, begin - 1, end - begin + 1);
	}

	if (end == std::string::npos)
		return;
	// param
	if (s[end] == '?') {
		begin = end + 1;
		end = s.find_first_of("#", begin);
		param(s.substr(begin, end - begin));
	}

	if (end == std::string::npos)
		return;
	// fragment
	begin = end + 1;
	fragment_ = s.substr(begin);
	//fragment_.assign(s, begin);
}

url& url::operator=(const std::string& s) {
	assign(s);
	return *this;
}

bool url::operator==(const url& u) {
	std::string rhss = this->get();
	std::string lhss = u.get();
	if( rhss == lhss )
		return true;
	else
		return false;
}

bool url::operator!=(const url& u) {
	return !(*this == u);
}

std::ostream& operator<<(std::ostream& os, const url& u) {
	if( u.has_userinfo() ) {
		os << "has_userinfo" << std::endl;
	}
	if( u.has_scheme() ) {
		os << "scheme: " << u.scheme_ << std::endl;
	}
	if( u.has_userinfo() ) {
		os << "userinfo: " << u.userinfo_ << std::endl;
	}
	if( !u.host_.empty() ) {
		os << "host: " << u.host_ << std::endl;
	}
	if( !u.port_.empty() ) {
		os << "port: " << u.port_ << std::endl;
	}
	if( u.has_path() ) {
		os << "path: " << u.path_ << std::endl;
	}
	if( u.has_param() ) {
		os << "param: " << u.param() << std::endl;
	}
	if( u.has_fragment() ) {
		os << "fragment: " << u.fragment_ << std::endl;
	}
	os << std::endl;
	return os;
}

void url::clear() {
	clear_scheme();
	clear_userinfo();
	clear_host();
	clear_port();
	clear_path();
	clear_param();
	clear_fragment();
}

bool url::empty() const
{
	if( has_scheme() || has_userinfo() || has_path() || has_param() || has_fragment() || !host_.empty() )
		return false;
	else
		return true;
}

void url::default_port() {
	if( scheme_ == "http")
		port_ = "80";
	else if ( scheme_ == "https")
		port_ = "443";
	else if ( scheme_ == "ftp")
		port_ = "21";
	else if ( scheme_ == "file")
		port_ = "21";
	else if ( scheme_ == "ssh")
		port_ = "22";
	else if ( scheme_ == "telnet")
		port_ = "23";
	else
		port_ = "80";
}

std::string url::get() const {
    std::string res;

    if(has_scheme())  {
        res += scheme();
        res += "://";
    }
    if(has_userinfo()) {
        res += userinfo_;
        res += "@";
    }
    res += host_ + ":" + port_;
    if (has_path()) {
    	res += path_;
	}
    if (has_param()) {
    	res += "?";
    	res += param();
	}
	if (!fragment_.empty()) {
    	res += "#";
    	res += fragment();
	}
    return res;
}

void url::erase_param(const std::string &key) {
	auto it = param_.find(key);
	if (it != param_.end()) {
		param_.erase(it);
	}
}

void url::param(const std::string &s) {
	param_.clear();
	std::vector<std::string> kv;

	boost::tokenizer<boost::char_separator<char> > tok(s, boost::char_separator<char>("&"));
	for (auto it = tok.begin(); it != tok.end(); ++it) {
		kv.clear();
		boost::split(kv, *it, boost::is_any_of("="));
		if (kv.size() == 1) {
			param_[url_decode(kv[0])] = "";
		} else {
			param_[url_decode(kv[0])] = url_decode(kv[1]);
		}
	}
}

std::string url::param() const {
	std::string s;
	for (auto it = param_.begin(); it != param_.end(); ++it) {
		if (!s.empty()) {
			s += "&";
		}
		s += it->first + "=" + it->second;
	}

	return s;
}

std::string url::uri() const {
	std::string s = path();
	if (s.empty()) {
		s = "/";
	}
	if (!param_.empty()) {
		s += "?" + param();
	}

	if (!fragment_.empty()) {
		s += "#" + fragment();
	}

	return s;
}

std::string url::url_decode(const std::string& str)
{
	auto const from_hex = [](unsigned char x) {
		unsigned char y;
		if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
		else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
		else if (x >= '0' && x <= '9') y = x - '0';
		else assert(0);
		return y;
    };

    std::string strTemp = "";
    size_t length = str.length();
    for (size_t i = 0; i < length; i++) {
        if (str[i] == '+') strTemp += ' ';
        else if (str[i] == '%') {
            assert(i + 2 < length);
            unsigned char high = from_hex((unsigned char)str[++i]);
            unsigned char low = from_hex((unsigned char)str[++i]);
            strTemp += high*16 + low;
        }
        else strTemp += str[i];
    }
    return strTemp;
}
