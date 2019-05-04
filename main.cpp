// Author: Sinipelto (c) 2019
// Dynamic IP address updater against domain API.
// Reads and parses configuration parameters from configuration file.

#include "ip.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <map>
#include <fstream>
#include <chrono>
#include <ctime>
#include <locale>
#include <curl/curl.h>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Option.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Form.hpp>

using namespace std;

map<string, string> ReadConfiguration(const string& path);

updater::ip ReadIpFromFile(const string& path);
updater::ip QueryIpFromUrl(const string &url, const bool &clean_result);

void UpdateIp(const updater::ip &ip, const string &url, const string &apikey, const string &apisecret);
void SaveIpToFile(const updater::ip &ip, const string &path);

std::pair<string, string> ParseLine(const string &line);
bool ParseBool(const string &input);

void WriteLog(const string &message);

void Terminate();

#if __WIN32
const string LogPath = "logs\\";
const string ConfigPath = "updater.conf";
const string LastIpPath = "lastip";
#else
const string LogPath = "logs/";
const string ConfigPath = "updater.conf";
const string LastIpPath = "lastip";
#endif

int main()
{
    //    curlpp::initialize(CURL_GLOBAL_ALL);

    map<string, string> config;

    try {
        config = ReadConfiguration(ConfigPath);
    } catch (const exception& e) {
        string msg = "";
        msg.append("Failed to read configuration: ");
        msg.append(e.what());
        WriteLog(msg);
        throw;
    }

    string domain = config["domain"];
    string api_key = config["api_key"];
    string api_secret = config["api_secret"];

    string ip_url = config["ip_url"];
    string api_url_base = config["api_url"];

    bool use_router_info = ParseBool(config["use_router_info"]);
    string router_url = config["router_url"];
    string router_username = config["router_username"];
    string router_password = config["router_password"];
    string router_keyword = config["router_keyword"];

    // Load last ip
    auto local_ip = ReadIpFromFile(LastIpPath);

    // Query current ip
    updater::ip remote_ip;

    if (use_router_info)
    {
        WriteLog("Using local router as external ip source..");
        remote_ip = QueryIpFromUrl(router_url, false);
    }
    else {
        WriteLog("Using external service for fetching ip address..");
        remote_ip = QueryIpFromUrl(ip_url, true);
    }

    // Compare IPs
    if (local_ip == remote_ip) {
        WriteLog("IP not changed, exiting without changes.");
        Terminate();
        return 0;
    }

    WriteLog("IP address changed from last record, updating the new ip..");
    WriteLog("Old IP: " + local_ip.toString());
    WriteLog("New IP: " + remote_ip.toString());

    // Create api url
    auto api_url = api_url_base + "/" + domain + "/records/A/@";

    // If changed, update to server with new ip
    UpdateIp(remote_ip, api_url, api_key, api_secret);

    WriteLog("IP updated to server successfully.");

    // Write down new ip to file
    SaveIpToFile(remote_ip, LastIpPath);

    WriteLog("Local IP record updated successfully.");

    // Terminate curlpp
    Terminate();

    return 0;
}

bool ParseBool(const string &input)
{
    string parsed = "";

    for (char c : input) {
        if (c == ' ' || c == '\n' || c == '\t' || c == '\r') continue;
        parsed += std::tolower(c, *new locale());
    }

    if (parsed == "")
    {
        WriteLog("Error: Could not parse boolean value for: " + input + "in configuration.");
        throw new exception;
    }

    if (parsed == "yes" || parsed == "y" || parsed == "true") return true;

    return false;
}

void Terminate()
{
    curlpp::terminate();
}

void WriteLog(const string &message)
{
    // Set time parse format string
    const char *fileFormat = "%Y-%m-%d";
    const char *stampFormat = "[%Y-%m-%d %H:%M:%S] ";

    // Get current time
    std::time_t now = std::time(nullptr);

    std::ostringstream ostring;

    ostring << std::put_time(std::localtime(&now), fileFormat);

    string logFileName;
    logFileName.append(LogPath);
    logFileName.append(ostring.str());
    logFileName.append(".log");

    ostring.seekp(0);

    ofstream logFile;
    logFile.open(logFileName, ios::app);

    if (logFile.fail())
    {
        cout << "Failed to open log file. Trying to create new one." << endl;
        logFile.open(logFileName);

        if (logFile.fail())
        {
            cout << "Failed to create log file, aborting.." << endl;
            throw new exception;
        }
    }

    ostring << std::put_time(std::localtime(&now), stampFormat) << message;

    logFile << ostring.str() << endl;

    cout << std::put_time(std::localtime(&now), stampFormat) << message << endl;
}


std::pair<string, string> ParseLine(const string &line)
{
    std::pair<string, string> keyvalue;

    for (unsigned i = 0; i < line.size(); i++)
    {
        if (line[i] == '=')
        {
            keyvalue.first = line.substr(0, i);
            keyvalue.second = line.substr(i+1, line.size()-1);
        }
    }

    return keyvalue;
}

map<string, string> ReadConfiguration(const string& path)
{
    map<string, string> settings = {};

    ifstream file;
    file.open(path);

    if (file.fail())
    {
        WriteLog("Could not open configuration file (updater.conf).");
        throw new exception;
    }

    string line;
    while (getline(file, line))
    {
        if (line[0] == '#') continue;
        else if (line.size() < 3) continue; // Format: a=b is minimum of 3

        auto parsed = ParseLine(line);
        settings[parsed.first] = parsed.second;
    }

    file.close();

    return settings;
}

updater::ip ReadIpFromFile(const string& path)
{
    ifstream file;
    file.open(path);

    if (file.fail())
    {
        WriteLog("Failed to open IP record file. Trying to create new one with null record.");

        ofstream out;
        out.open(path);

        if (out.fail()) {
            WriteLog("Failed to create ip record. Cannot continue.");
            throw new exception;
        }

        out << "0.0.0.0";
        out.close();

        WriteLog("New IP record file written, continuing as normal.");

        file.open(path);
    }

    string line;
    getline(file, line);
    file.close();

    updater::ip *addr;

    try {
        addr = new updater::ip(line);
    } catch (const std::exception &e) {
        string msg = "";
        msg.append("Error while parsing last ip from file: ");
        msg.append(e.what());
        WriteLog(msg);
        throw;
    }

    return *addr;
}

void UpdateIp(const updater::ip &ip, const string &url, const string &apikey, const string &apisecret)
{
    curlpp::Easy handle;

    std::stringstream buffer;

    auto authOpt = curlpp::options::HttpHeader({"Content-Type: application/json", "Authorization: sso-key " + apikey + ":" + apisecret});
    auto dataOpt = curlpp::Options::PostFields("[{\"data\":\"" + ip.toString() + "\"}]");

    handle.setOpt(curlpp::Options::Url(url));
    handle.setOpt(curlpp::Options::CustomRequest("PUT"));
    handle.setOpt(authOpt);
    handle.setOpt(dataOpt);
    handle.setOpt(curlpp::Options::SslVerifyHost(false));
    handle.setOpt(curlpp::Options::SslVerifyPeer(false));
    handle.setOpt(curlpp::Options::WriteStream(&buffer));

    handle.perform();

    string resp;
    string strBuf;

    while (buffer >> strBuf) {
        resp += strBuf + " ";
    }

    if (resp.length() > 10)
    {
        WriteLog("Error in update: " + resp);
        throw new exception;
    }
}

updater::ip QueryIpFromUrl(const string &url, const bool &clean_result)
{
    if (!clean_result)
    {
        WriteLog("Unclean ip fetching not yet implemented. Please use clean source for fetching ip address.");
        throw new exception;
    }

    curlpp::Easy handle;

    std::stringstream buffer;

    handle.setOpt(curlpp::Options::Url(url));
    handle.setOpt(curlpp::Options::HttpGet(true));
    handle.setOpt(curlpp::Options::SslVerifyHost(false));
    handle.setOpt(curlpp::Options::SslVerifyPeer(false));
    handle.setOpt(curlpp::Options::WriteStream(&buffer));

    handle.perform();

    string resp;

    buffer >> resp;

    updater::ip *addr;

    try {
        addr = new updater::ip(resp);
    } catch (const std::exception &e) {
        string msg = "";
        msg.append("Error while parsing current remote ip: ");
        msg.append(e.what());
        WriteLog(msg);
        throw;
    }

    return *addr;
}

void SaveIpToFile(const updater::ip &ip, const string &path)
{
    ofstream file;
    file.open(path, ios::out);

    if (file.fail()) {
        WriteLog("Failed to open ip record file. Ensure necessary resources are available for operation.");
        throw new exception;
    }

    file << ip.toString();

    file.close();
}
