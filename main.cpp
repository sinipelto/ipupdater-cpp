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

const string *ProcessPath(string rawPath);

map<string, string> *ReadConfiguration(const string * const path);

const updater::ip *ReadIpFromFile(const string * const path);
const updater::ip *QueryIpFromUrl(const string &url, const bool &clean_result);

void UpdateIp(const updater::ip * const ip, const string &url, const string &apikey, const string &apisecret);
void SaveIpToFile(const updater::ip * const ip, const string * const path);

std::pair<string, string> ParseLine(const string &line);
bool ParseBool(const string &input);

void WriteLog(const string &message);

int Terminate();

#define WRITE_EXIT WriteLog("----------------------------------------------------------------")

#if __WIN32
const char DELIM = '\\';
static string LogPath = "logs\\";
#else
const char DELIM = '/';
static string LogPath = "logs/";
#endif


int main(int argc, char **argv)
{
#if __WIN32
    const string * const basePath = new string("");
#else
    const string * const basePath = ProcessPath(argv[0]);
#endif
    const string * const configPath = new string(*basePath + "updater.conf");
    const string * const lastIpPath = new string(*basePath + "lastip");
    LogPath = *basePath + LogPath;

    delete basePath;

    map<string, string> *config;

    try {
        config = ReadConfiguration(configPath);
    } catch (const exception& e) {
        string msg = "";
        msg.append("Failed to read configuration: ");
        msg.append(e.what());
        WriteLog(msg);
        WRITE_EXIT;
        throw;
    }

    delete configPath;

    const string domain = config->at("domain");
    const string api_key = config->at("api_key");
    const string api_secret = config->at("api_secret");

    const string ip_url = config->at("ip_url");
    const string api_url_base = config->at("api_url");

    const bool use_router_info = ParseBool(config->at("use_router_info"));
    const string router_url = config->at("router_url");
    const string router_username = config->at("router_username");
    const string router_password = config->at("router_password");
    const string router_keyword = config->at("router_keyword");

    // Free configuration map memory
    delete config;

    // Load last ip
    auto local_ip = ReadIpFromFile(lastIpPath);

    // Query current ip
    const updater::ip *remote_ip;

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

        delete lastIpPath;
        delete local_ip;
        delete remote_ip;

        WRITE_EXIT;
        return Terminate();
    }

    WriteLog("IP address changed from last record, updating the new ip..");
    WriteLog("Old IP: " + local_ip->toString());
    WriteLog("New IP: " + remote_ip->toString());

    delete local_ip;

    // Create api url
    const string &api_url = api_url_base + "/" + domain + "/records/A/@";

    // If changed, update to server with new ip
    UpdateIp(remote_ip, api_url, api_key, api_secret);

    WriteLog("IP updated to server successfully.");

    // Write down new ip to file
    SaveIpToFile(remote_ip, lastIpPath);
    delete remote_ip;
    delete lastIpPath;

    WriteLog("Local IP record updated successfully.");

    // Terminate curlpp
    WRITE_EXIT;
    return Terminate();
}

const string *ProcessPath(string rawPath)
{
    auto it = rawPath.rbegin();

    while (*it != DELIM)
    {
        rawPath.pop_back();
        it++;
    }

    return new string(rawPath);
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

int Terminate()
{
    curlpp::terminate();
    return 0;
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

map<string, string> *ReadConfiguration(const string * const path)
{
    map<string, string> *settings = new map<string, string>();

    ifstream file;
    file.open(*path);

    if (file.fail())
    {
        WriteLog("Could not open configuration file (updater.conf).");
        delete path;
        WRITE_EXIT;
        throw new exception;
    }

    string line;
    while (getline(file, line))
    {
        if (line[0] == '#') continue;
        else if (line.size() < 3) continue; // Format: a=b is minimum of 3

        auto parsed = ParseLine(line);
        settings->operator[](parsed.first) = parsed.second;
    }

    file.close();

    return settings;
}

const updater::ip *ReadIpFromFile(const string * const path)
{
    ifstream file;
    file.open(*path);

    if (file.fail())
    {
        WriteLog("Failed to open IP record file. Trying to create new one with null record.");

        ofstream out;
        out.open(*path);

        if (out.fail()) {
            WriteLog("Failed to create ip record. Cannot continue.");
            delete path;
            WRITE_EXIT;
            throw new exception;
        }

        out << "0.0.0.0";
        out.close();

        WriteLog("New IP record file written, continuing as normal.");

        file.open(*path);
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
        WRITE_EXIT;
        throw;
    }

    return addr;
}

void UpdateIp(const updater::ip * const ip, const string &url, const string &apikey, const string &apisecret)
{
    curlpp::Easy handle;

    std::stringstream buffer;

    auto authOpt = curlpp::options::HttpHeader({"Content-Type: application/json", "Authorization: sso-key " + apikey + ":" + apisecret});
    auto dataOpt = curlpp::Options::PostFields("[{\"data\":\"" + ip->toString() + "\"}]");

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
        delete ip;
        WRITE_EXIT;
        throw new exception;
    }
}

const updater::ip *QueryIpFromUrl(const string &url, const bool &clean_result)
{
    if (!clean_result)
    {
        WriteLog("Unclean ip fetching not yet implemented. Please use clean source for fetching ip address.");
        WRITE_EXIT;
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
        WRITE_EXIT;
        throw;
    }

    return addr;
}

void SaveIpToFile(const updater::ip * const ip, const string * const path)
{
    ofstream file;
    file.open(*path, ios::out);

    if (file.fail()) {
        WriteLog("Failed to open ip record file. Ensure necessary resources are available for operation.");
        delete path;
        WRITE_EXIT;
        throw new exception;
    }

    file << ip->toString();
    file.close();
}
