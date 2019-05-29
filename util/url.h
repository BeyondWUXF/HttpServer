/**
 * @class url url.h
 * @brief handles URLs
 *
 * 实现简单的url解析
 *
 * \verbatim
 * See RFC 3986.
 * http://user:password@host:port/path
 * foo://example.com:8042/over/there?name=ferret#nose
 * \_/   \______________/\_________/ \_________/ \__/
 *  |           |            |           |        |
 * scheme     authority     path      param     fragment
 *        \_______________________/
 *                  |
 *              hier_part
 */

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

class url {
public:
	explicit url(const std::string& s);
	explicit url();
	~url();

	void assign(const std::string& s);

	/*** OPERATORS ***/
	url& operator=(const std::string& s);

	bool operator==(const url& u);
	bool operator!=(const url& u);

	operator std::string() { return get(); }

	friend std::ostream& operator<<(std::ostream& os, const url& u);

	void clear();

	bool empty() const;

	/**
	* Get the whole url as std::string
	*/
	std::string get() const;
	std::string to_string() const { return get(); };
	std::string as_string() const { return get(); };

	/***** ACCESSORS *****/
	inline void scheme(const std::string& s) { scheme_ = s; }
	inline std::string scheme() const { return scheme_; }
	inline void clear_scheme() { scheme_.clear(); }
	inline bool has_scheme() const { return !scheme_.empty(); }

	inline void userinfo(const std::string& s) { userinfo_ = s; }
	inline std::string userinfo() const { return userinfo_; }
	inline bool has_userinfo() const { return !userinfo_.empty(); }
	inline void clear_userinfo() { userinfo_.clear(); }

	inline void host(const std::string& s) { host_ = s; }
	inline std::string host() const { return host_; }
	inline void clear_host() { host_.clear(); }

	inline void port(const std::string& s) { port_ = s; }
	inline std::string port() const { return port_; }
	inline int port_int() const { return std::stoi(port_); }
	inline void clear_port() { port_.clear(); }

	inline void path(const std::string& s) { path_ = s; }
	inline std::string path() const { return path_; }
	inline void clear_path() { path_.clear(); }
	inline bool has_path() const { return !path_.empty(); }

	inline void param(const std::map<std::string, std::string> &m) { param_ = m; }
	inline bool has_param() const { return !param_.empty(); }
	inline void clear_param() { param_.clear(); }
	inline void add_param(const std::string &key, const std::string &value) { param_[key] = value; }
	inline std::string get_param(const std::string &key) { return param_[key]; }
	void erase_param(const std::string &key);
	void param(const std::string &s);
	std::string param() const;
	std::string uri() const;

	inline void fragment(const std::string& s) { fragment_ = s; }
	inline std::string fragment() const { return fragment_; }
	inline void clear_fragment() { fragment_.clear(); }
	inline bool has_fragment() const { return !fragment_.empty(); }

private:
	std::string url_decode(const std::string& str);
	void default_port();

    std::string scheme_;
    std::string userinfo_;
    std::string host_;
    std::string port_;
    std::string path_;
    std::map<std::string, std::string> param_;
    std::string fragment_;
};

