//
// Created by fred on 11/12/16.
//

#include <iostream>
#include <sstream>
#include <algorithm>
#include "frnetlib/Http.h"

namespace fr
{
    const static std::string request_type_strings[Http::RequestType::RequestTypeCount] = {"UNKNOWN", "GET", "POST"};
    std::unordered_map<std::string, std::string> Http::mime_types;

    Http::Http()
    {
        if(mime_types.empty())
            load_mimetypes();
        clear();
    }

    Http::Http(Http &&o)
    : header_data(std::move(o.header_data)),
      post_data(std::move(o.post_data)),
      get_data(std::move(o.get_data)),
      body(std::move(o.body)),
      request_type(o.request_type),
      uri(std::move(o.uri)),
      status(o.status)
    {
    }

    Http::Http(const Http &o)
    : header_data(o.header_data),
      post_data(o.post_data),
      get_data(o.get_data),
      body(o.body),
      request_type(o.request_type),
      uri(o.uri),
      status(o.status)
    {

    }

    Http::RequestType Http::get_type() const
    {
        return request_type;
    }

    std::vector<std::string> Http::split_string(const std::string &str)
    {
        char last_character = '\0';
        size_t line_start = 0;
        std::vector<std::string> result;

        for(size_t a = 0; a < str.size(); a++)
        {
            if(str[a] == '\n' && last_character != '\\')
            {
                result.emplace_back(str.substr(line_start, a - line_start));
                line_start = a + 1;
            }
            last_character = str[a];
        }
        result.emplace_back(str.substr(line_start, str.size() - line_start));
        return result;
    }

    void Http::set_body(const std::string &body_)
    {
        body = body_;
    }

    void Http::clear()
    {
        post_data.clear();
        get_data.clear();
        post_data.clear();
        body.clear();
        uri = "/";
        status = Ok;
        request_type = Unknown;
    }

    std::string &Http::get(const std::string &key)
    {
        return get_data[key];
    }

    std::string &Http::post(const std::string &key)
    {
        return post_data[key];
    }

    bool Http::get_exists(const std::string &key) const
    {
        return get_data.find(key) != get_data.end();
    }

    bool Http::post_exists(const std::string &key) const
    {
        return post_data.find(key) != post_data.end();
    }

    const std::string &Http::get_uri() const
    {
        return uri;
    }

    void Http::set_status(RequestStatus status_)
    {
        status = status_;
    }

    Http::RequestStatus Http::get_status() const
    {
        return status;
    }

    void Http::set_uri(const std::string &str)
    {
        //Don't do anything if there's no URI provided
        if(str.empty())
            return;

        //Ensure that URI begins with a /
        if(str.front() == '/')
            uri = str;
        else
            uri = "/" + str;
    }

    std::string Http::request_type_to_string(RequestType type) const
    {
        if(type >= RequestType::RequestTypeCount)
            return request_type_strings[0];
        return request_type_strings[type];
    }

    void Http::set_type(Http::RequestType type)
    {
        request_type = type;
    }

    const std::string &Http::get_body() const
    {
        return body;
    }

    std::string Http::url_encode(const std::string &str)
    {
        std::stringstream encoded;
        encoded << std::hex;
        for(const auto &c : str)
        {
            if(isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
                encoded << c;
            else if(c == ' ')
                encoded << '+';
            else
                encoded << "%" << std::uppercase << (int)c << std::nouppercase;
        }
        return encoded.str();
    }

    std::string Http::url_decode(const std::string &str)
    {
        std::string result;
        for(size_t a = 0; a < str.size(); a++)
        {
            if(str[a] == '%' && a < str.size() - 1)
            {
                result += (char)dectohex(str.substr(a + 1, 2));
                a += 2;
            }
            else if(str[a] == '+')
            {
                result += " ";
            }
            else
            {
                result += str[a];
            }
        }
        return result;
    }

    std::string &Http::header(std::string &&key)
    {
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        return header_data[key];
    }

    bool Http::header_exists(const std::string &key) const
    {
        return header_data.find(key) != header_data.end();
    }

    std::vector<std::pair<std::string, std::string>> Http::parse_argument_list(const std::string &str)
    {
        std::vector<std::pair<std::string, std::string>> list;
        if(str.empty())
            return list;

        size_t read_index = 0;
        if(str.front() == '?')
            read_index++;

        while(true)
        {
            auto equal_pos = str.find("=", read_index);
            if(equal_pos != std::string::npos)
            {
                auto and_pos = str.find("&", read_index);
                if(and_pos == std::string::npos)
                {
                    list.emplace_back(str.substr(read_index, equal_pos - read_index), str.substr(equal_pos + 1, str.size() - equal_pos - 1));
                    break;
                }
                else
                {
                    list.emplace_back(str.substr(read_index, equal_pos - read_index), str.substr(equal_pos + 1, and_pos - equal_pos - 1));
                    read_index = and_pos + 1;
                }
            }
            else
            {
                break;
            }
        }

        return list;
    }

    void Http::parse_header_line(const std::string &str)
    {
        size_t colon_pos = str.find(':');
        if(colon_pos == std::string::npos)
            return;

        auto data_begin = str.find_first_not_of(" ", colon_pos + 1);
        if(data_begin == std::string::npos)
            return;

        size_t data_len = 0;
        for(size_t a = data_begin; a < str.size(); a++)
        {
            if(str[a] >= 32 && str[a] <= 126)
                data_len++;
            else
                break;
        }
        std::string header_name = str.substr(0, colon_pos);
        std::string header_value = str.substr(data_begin, data_len);

        std::transform(header_name.begin(), header_name.end(), header_name.begin(), ::tolower);
        header_data.emplace(std::move(header_name), std::move(header_value));
    }

    void Http::load_mimetypes()
    {
        mime_types.emplace(".x3d", "application/vnd.hzn-3d-crossword");
        mime_types.emplace(".3gp", "video/3gpp");
        mime_types.emplace(".3g2", "video/3gpp2");
        mime_types.emplace(".mseq", "application/vnd.mseq");
        mime_types.emplace(".pwn", "application/vnd.3m.post-it-notes");
        mime_types.emplace(".plb", "application/vnd.3gpp.pic-bw-large");
        mime_types.emplace(".psb", "application/vnd.3gpp.pic-bw-small");
        mime_types.emplace(".pvb", "application/vnd.3gpp.pic-bw-var");
        mime_types.emplace(".tcap", "application/vnd.3gpp2.tcap");
        mime_types.emplace(".7z", "application/x-7z-compressed");
        mime_types.emplace(".abw", "application/x-abiword");
        mime_types.emplace(".ace", "application/x-ace-compressed");
        mime_types.emplace(".acc", "application/vnd.americandynamics.acc");
        mime_types.emplace(".acu", "application/vnd.acucobol");
        mime_types.emplace(".atc", "application/vnd.acucorp");
        mime_types.emplace(".adp", "audio/adpcm");
        mime_types.emplace(".aab", "application/x-authorware-bin");
        mime_types.emplace(".aam", "application/x-authorware-map");
        mime_types.emplace(".aas", "application/x-authorware-seg");
        mime_types.emplace(".air", "application/vnd.adobe.air-application-installer-package+zip");
        mime_types.emplace(".swf", "application/x-shockwave-flash");
        mime_types.emplace(".fxp", "application/vnd.adobe.fxp");
        mime_types.emplace(".pdf", "application/pdf");
        mime_types.emplace(".ppd", "application/vnd.cups-ppd");
        mime_types.emplace(".dir", "application/x-director");
        mime_types.emplace(".xdp", "application/vnd.adobe.xdp+xml");
        mime_types.emplace(".xfdf", "application/vnd.adobe.xfdf");
        mime_types.emplace(".aac", "audio/x-aac");
        mime_types.emplace(".ahead", "application/vnd.ahead.space");
        mime_types.emplace(".azf", "application/vnd.airzip.filesecure.azf");
        mime_types.emplace(".azs", "application/vnd.airzip.filesecure.azs");
        mime_types.emplace(".azw", "application/vnd.amazon.ebook");
        mime_types.emplace(".ami", "application/vnd.amiga.ami");
        mime_types.emplace("N/A", "application/andrew-inset");
        mime_types.emplace(".apk", "application/vnd.android.package-archive");
        mime_types.emplace(".cii", "application/vnd.anser-web-certificate-issue-initiation");
        mime_types.emplace(".fti", "application/vnd.anser-web-funds-transfer-initiation");
        mime_types.emplace(".atx", "application/vnd.antix.game-component");
        mime_types.emplace(".mpkg", "application/vnd.apple.installer+xml");
        mime_types.emplace(".aw", "application/applixware");
        mime_types.emplace(".les", "application/vnd.hhe.lesson-player");
        mime_types.emplace(".swi", "application/vnd.aristanetworks.swi");
        mime_types.emplace(".s", "text/x-asm");
        mime_types.emplace(".atomcat", "application/atomcat+xml");
        mime_types.emplace(".atomsvc", "application/atomsvc+xml");
        mime_types.emplace(".atom, .xml", "application/atom+xml");
        mime_types.emplace(".ac", "application/pkix-attr-cert");
        mime_types.emplace(".aif", "audio/x-aiff");
        mime_types.emplace(".avi", "video/x-msvideo");
        mime_types.emplace(".aep", "application/vnd.audiograph");
        mime_types.emplace(".dxf", "image/vnd.dxf");
        mime_types.emplace(".dwf", "model/vnd.dwf");
        mime_types.emplace(".par", "text/plain-bas");
        mime_types.emplace(".bcpio", "application/x-bcpio");
        mime_types.emplace(".bin", "application/octet-stream");
        mime_types.emplace(".bmp", "image/bmp");
        mime_types.emplace(".torrent", "application/x-bittorrent");
        mime_types.emplace(".cod", "application/vnd.rim.cod");
        mime_types.emplace(".mpm", "application/vnd.blueice.multipass");
        mime_types.emplace(".bmi", "application/vnd.bmi");
        mime_types.emplace(".sh", "application/x-sh");
        mime_types.emplace(".btif", "image/prs.btif");
        mime_types.emplace(".rep", "application/vnd.businessobjects");
        mime_types.emplace(".bz", "application/x-bzip");
        mime_types.emplace(".bz2", "application/x-bzip2");
        mime_types.emplace(".csh", "application/x-csh");
        mime_types.emplace(".c", "text/x-c");
        mime_types.emplace(".cdxml", "application/vnd.chemdraw+xml");
        mime_types.emplace(".css", "text/css");
        mime_types.emplace(".cdx", "chemical/x-cdx");
        mime_types.emplace(".cml", "chemical/x-cml");
        mime_types.emplace(".csml", "chemical/x-csml");
        mime_types.emplace(".cdbcmsg", "application/vnd.contact.cmsg");
        mime_types.emplace(".cla", "application/vnd.claymore");
        mime_types.emplace(".c4g", "application/vnd.clonk.c4group");
        mime_types.emplace(".sub", "image/vnd.dvb.subtitle");
        mime_types.emplace(".cdmia", "application/cdmi-capability");
        mime_types.emplace(".cdmic", "application/cdmi-container");
        mime_types.emplace(".cdmid", "application/cdmi-domain");
        mime_types.emplace(".cdmio", "application/cdmi-object");
        mime_types.emplace(".cdmiq", "application/cdmi-queue");
        mime_types.emplace(".c11amc", "application/vnd.cluetrust.cartomobile-config");
        mime_types.emplace(".c11amz", "application/vnd.cluetrust.cartomobile-config-pkg");
        mime_types.emplace(".ras", "image/x-cmu-raster");
        mime_types.emplace(".dae", "model/vnd.collada+xml");
        mime_types.emplace(".csv", "text/csv");
        mime_types.emplace(".cpt", "application/mac-compactpro");
        mime_types.emplace(".wmlc", "application/vnd.wap.wmlc");
        mime_types.emplace(".cgm", "image/cgm");
        mime_types.emplace(".ice", "x-conference/x-cooltalk");
        mime_types.emplace(".cmx", "image/x-cmx");
        mime_types.emplace(".xar", "application/vnd.xara");
        mime_types.emplace(".cmc", "application/vnd.cosmocaller");
        mime_types.emplace(".cpio", "application/x-cpio");
        mime_types.emplace(".clkx", "application/vnd.crick.clicker");
        mime_types.emplace(".clkk", "application/vnd.crick.clicker.keyboard");
        mime_types.emplace(".clkp", "application/vnd.crick.clicker.palette");
        mime_types.emplace(".clkt", "application/vnd.crick.clicker.template");
        mime_types.emplace(".clkw", "application/vnd.crick.clicker.wordbank");
        mime_types.emplace(".wbs", "application/vnd.criticaltools.wbs+xml");
        mime_types.emplace(".cryptonote", "application/vnd.rig.cryptonote");
        mime_types.emplace(".cif", "chemical/x-cif");
        mime_types.emplace(".cmdf", "chemical/x-cmdf");
        mime_types.emplace(".cu", "application/cu-seeme");
        mime_types.emplace(".cww", "application/prs.cww");
        mime_types.emplace(".curl", "text/vnd.curl");
        mime_types.emplace(".dcurl", "text/vnd.curl.dcurl");
        mime_types.emplace(".mcurl", "text/vnd.curl.mcurl");
        mime_types.emplace(".scurl", "text/vnd.curl.scurl");
        mime_types.emplace(".car", "application/vnd.curl.car");
        mime_types.emplace(".pcurl", "application/vnd.curl.pcurl");
        mime_types.emplace(".cmp", "application/vnd.yellowriver-custom-menu");
        mime_types.emplace(".dssc", "application/dssc+der");
        mime_types.emplace(".xdssc", "application/dssc+xml");
        mime_types.emplace(".deb", "application/x-debian-package");
        mime_types.emplace(".uva", "audio/vnd.dece.audio");
        mime_types.emplace(".uvi", "image/vnd.dece.graphic");
        mime_types.emplace(".uvh", "video/vnd.dece.hd");
        mime_types.emplace(".uvm", "video/vnd.dece.mobile");
        mime_types.emplace(".uvu", "video/vnd.uvvu.mp4");
        mime_types.emplace(".uvp", "video/vnd.dece.pd");
        mime_types.emplace(".uvs", "video/vnd.dece.sd");
        mime_types.emplace(".uvv", "video/vnd.dece.video");
        mime_types.emplace(".dvi", "application/x-dvi");
        mime_types.emplace(".seed", "application/vnd.fdsn.seed");
        mime_types.emplace(".dtb", "application/x-dtbook+xml");
        mime_types.emplace(".res", "application/x-dtbresource+xml");
        mime_types.emplace(".ait", "application/vnd.dvb.ait");
        mime_types.emplace(".svc", "application/vnd.dvb.service");
        mime_types.emplace(".eol", "audio/vnd.digital-winds");
        mime_types.emplace(".djvu", "image/vnd.djvu");
        mime_types.emplace(".dtd", "application/xml-dtd");
        mime_types.emplace(".mlp", "application/vnd.dolby.mlp");
        mime_types.emplace(".wad", "application/x-doom");
        mime_types.emplace(".dpg", "application/vnd.dpgraph");
        mime_types.emplace(".dra", "audio/vnd.dra");
        mime_types.emplace(".dfac", "application/vnd.dreamfactory");
        mime_types.emplace(".dts", "audio/vnd.dts");
        mime_types.emplace(".dtshd", "audio/vnd.dts.hd");
        mime_types.emplace(".dwg", "image/vnd.dwg");
        mime_types.emplace(".geo", "application/vnd.dynageo");
        mime_types.emplace(".es", "application/ecmascript");
        mime_types.emplace(".mag", "application/vnd.ecowin.chart");
        mime_types.emplace(".mmr", "image/vnd.fujixerox.edmics-mmr");
        mime_types.emplace(".rlc", "image/vnd.fujixerox.edmics-rlc");
        mime_types.emplace(".exi", "application/exi");
        mime_types.emplace(".mgz", "application/vnd.proteus.magazine");
        mime_types.emplace(".epub", "application/epub+zip");
        mime_types.emplace(".eml", "message/rfc822");
        mime_types.emplace(".nml", "application/vnd.enliven");
        mime_types.emplace(".xpr", "application/vnd.is-xpr");
        mime_types.emplace(".xif", "image/vnd.xiff");
        mime_types.emplace(".xfdl", "application/vnd.xfdl");
        mime_types.emplace(".emma", "application/emma+xml");
        mime_types.emplace(".ez2", "application/vnd.ezpix-album");
        mime_types.emplace(".ez3", "application/vnd.ezpix-package");
        mime_types.emplace(".fst", "image/vnd.fst");
        mime_types.emplace(".fvt", "video/vnd.fvt");
        mime_types.emplace(".fbs", "image/vnd.fastbidsheet");
        mime_types.emplace(".fe_launch", "application/vnd.denovo.fcselayout-link");
        mime_types.emplace(".f4v", "video/x-f4v");
        mime_types.emplace(".flv", "video/x-flv");
        mime_types.emplace(".fpx", "image/vnd.fpx");
        mime_types.emplace(".npx", "image/vnd.net-fpx");
        mime_types.emplace(".flx", "text/vnd.fmi.flexstor");
        mime_types.emplace(".fli", "video/x-fli");
        mime_types.emplace(".ftc", "application/vnd.fluxtime.clip");
        mime_types.emplace(".fdf", "application/vnd.fdf");
        mime_types.emplace(".f", "text/x-fortran");
        mime_types.emplace(".mif", "application/vnd.mif");
        mime_types.emplace(".fm", "application/vnd.framemaker");
        mime_types.emplace(".fh", "image/x-freehand");
        mime_types.emplace(".fsc", "application/vnd.fsc.weblaunch");
        mime_types.emplace(".fnc", "application/vnd.frogans.fnc");
        mime_types.emplace(".ltf", "application/vnd.frogans.ltf");
        mime_types.emplace(".ddd", "application/vnd.fujixerox.ddd");
        mime_types.emplace(".xdw", "application/vnd.fujixerox.docuworks");
        mime_types.emplace(".xbd", "application/vnd.fujixerox.docuworks.binder");
        mime_types.emplace(".oas", "application/vnd.fujitsu.oasys");
        mime_types.emplace(".oa2", "application/vnd.fujitsu.oasys2");
        mime_types.emplace(".oa3", "application/vnd.fujitsu.oasys3");
        mime_types.emplace(".fg5", "application/vnd.fujitsu.oasysgp");
        mime_types.emplace(".bh2", "application/vnd.fujitsu.oasysprs");
        mime_types.emplace(".spl", "application/x-futuresplash");
        mime_types.emplace(".fzs", "application/vnd.fuzzysheet");
        mime_types.emplace(".g3", "image/g3fax");
        mime_types.emplace(".gmx", "application/vnd.gmx");
        mime_types.emplace(".gtw", "model/vnd.gtw");
        mime_types.emplace(".txd", "application/vnd.genomatix.tuxedo");
        mime_types.emplace(".ggb", "application/vnd.geogebra.file");
        mime_types.emplace(".ggt", "application/vnd.geogebra.tool");
        mime_types.emplace(".gdl", "model/vnd.gdl");
        mime_types.emplace(".gex", "application/vnd.geometry-explorer");
        mime_types.emplace(".gxt", "application/vnd.geonext");
        mime_types.emplace(".g2w", "application/vnd.geoplan");
        mime_types.emplace(".g3w", "application/vnd.geospace");
        mime_types.emplace(".gsf", "application/x-font-ghostscript");
        mime_types.emplace(".bdf", "application/x-font-bdf");
        mime_types.emplace(".gtar", "application/x-gtar");
        mime_types.emplace(".texinfo", "application/x-texinfo");
        mime_types.emplace(".gnumeric", "application/x-gnumeric");
        mime_types.emplace(".kml", "application/vnd.google-earth.kml+xml");
        mime_types.emplace(".kmz", "application/vnd.google-earth.kmz");
        mime_types.emplace(".gqf", "application/vnd.grafeq");
        mime_types.emplace(".gif", "image/gif");
        mime_types.emplace(".gv", "text/vnd.graphviz");
        mime_types.emplace(".gac", "application/vnd.groove-account");
        mime_types.emplace(".ghf", "application/vnd.groove-help");
        mime_types.emplace(".gim", "application/vnd.groove-identity-message");
        mime_types.emplace(".grv", "application/vnd.groove-injector");
        mime_types.emplace(".gtm", "application/vnd.groove-tool-message");
        mime_types.emplace(".tpl", "application/vnd.groove-tool-template");
        mime_types.emplace(".vcg", "application/vnd.groove-vcard");
        mime_types.emplace(".h261", "video/h261");
        mime_types.emplace(".h263", "video/h263");
        mime_types.emplace(".h264", "video/h264");
        mime_types.emplace(".hpid", "application/vnd.hp-hpid");
        mime_types.emplace(".hps", "application/vnd.hp-hps");
        mime_types.emplace(".hdf", "application/x-hdf");
        mime_types.emplace(".rip", "audio/vnd.rip");
        mime_types.emplace(".hbci", "application/vnd.hbci");
        mime_types.emplace(".jlt", "application/vnd.hp-jlyt");
        mime_types.emplace(".pcl", "application/vnd.hp-pcl");
        mime_types.emplace(".hpgl", "application/vnd.hp-hpgl");
        mime_types.emplace(".hvs", "application/vnd.yamaha.hv-script");
        mime_types.emplace(".hvd", "application/vnd.yamaha.hv-dic");
        mime_types.emplace(".hvp", "application/vnd.yamaha.hv-voice");
        mime_types.emplace(".sfd-hdstx", "application/vnd.hydrostatix.sof-data");
        mime_types.emplace(".stk", "application/hyperstudio");
        mime_types.emplace(".hal", "application/vnd.hal+xml");
        mime_types.emplace(".html", "text/html");
        mime_types.emplace(".irm", "application/vnd.ibm.rights-management");
        mime_types.emplace(".sc", "application/vnd.ibm.secure-container");
        mime_types.emplace(".ics", "text/calendar");
        mime_types.emplace(".icc", "application/vnd.iccprofile");
        mime_types.emplace(".ico", "image/x-icon");
        mime_types.emplace(".igl", "application/vnd.igloader");
        mime_types.emplace(".ief", "image/ief");
        mime_types.emplace(".ivp", "application/vnd.immervision-ivp");
        mime_types.emplace(".ivu", "application/vnd.immervision-ivu");
        mime_types.emplace(".rif", "application/reginfo+xml");
        mime_types.emplace(".3dml", "text/vnd.in3d.3dml");
        mime_types.emplace(".spot", "text/vnd.in3d.spot");
        mime_types.emplace(".igs", "model/iges");
        mime_types.emplace(".i2g", "application/vnd.intergeo");
        mime_types.emplace(".cdy", "application/vnd.cinderella");
        mime_types.emplace(".xpw", "application/vnd.intercon.formnet");
        mime_types.emplace(".fcs", "application/vnd.isac.fcs");
        mime_types.emplace(".ipfix", "application/ipfix");
        mime_types.emplace(".cer", "application/pkix-cert");
        mime_types.emplace(".pki", "application/pkixcmp");
        mime_types.emplace(".crl", "application/pkix-crl");
        mime_types.emplace(".pkipath", "application/pkix-pkipath");
        mime_types.emplace(".igm", "application/vnd.insors.igm");
        mime_types.emplace(".rcprofile", "application/vnd.ipunplugged.rcprofile");
        mime_types.emplace(".irp", "application/vnd.irepository.package+xml");
        mime_types.emplace(".jad", "text/vnd.sun.j2me.app-descriptor");
        mime_types.emplace(".jar", "application/java-archive");
        mime_types.emplace(".class", "application/java-vm");
        mime_types.emplace(".jnlp", "application/x-java-jnlp-file");
        mime_types.emplace(".ser", "application/java-serialized-object");
        mime_types.emplace(".java", "text/x-java-source,java");
        mime_types.emplace(".js", "application/javascript");
        mime_types.emplace(".json", "application/json");
        mime_types.emplace(".joda", "application/vnd.joost.joda-archive");
        mime_types.emplace(".jpm", "video/jpm");
        mime_types.emplace(".jpeg, .jpg", "image/jpeg");
        mime_types.emplace(".jpeg, .jpg", "image/x-citrix-jpeg");
        mime_types.emplace(".pjpeg", "image/pjpeg");
        mime_types.emplace(".jpgv", "video/jpeg");
        mime_types.emplace(".ktz", "application/vnd.kahootz");
        mime_types.emplace(".mmd", "application/vnd.chipnuts.karaoke-mmd");
        mime_types.emplace(".karbon", "application/vnd.kde.karbon");
        mime_types.emplace(".chrt", "application/vnd.kde.kchart");
        mime_types.emplace(".kfo", "application/vnd.kde.kformula");
        mime_types.emplace(".flw", "application/vnd.kde.kivio");
        mime_types.emplace(".kon", "application/vnd.kde.kontour");
        mime_types.emplace(".kpr", "application/vnd.kde.kpresenter");
        mime_types.emplace(".ksp", "application/vnd.kde.kspread");
        mime_types.emplace(".kwd", "application/vnd.kde.kword");
        mime_types.emplace(".htke", "application/vnd.kenameaapp");
        mime_types.emplace(".kia", "application/vnd.kidspiration");
        mime_types.emplace(".kne", "application/vnd.kinar");
        mime_types.emplace(".sse", "application/vnd.kodak-descriptor");
        mime_types.emplace(".lasxml", "application/vnd.las.las+xml");
        mime_types.emplace(".latex", "application/x-latex");
        mime_types.emplace(".lbd", "application/vnd.llamagraphics.life-balance.desktop");
        mime_types.emplace(".lbe", "application/vnd.llamagraphics.life-balance.exchange+xml");
        mime_types.emplace(".jam", "application/vnd.jam");
        mime_types.emplace(".123", "application/vnd.lotus-1-2-3");
        mime_types.emplace(".apr", "application/vnd.lotus-approach");
        mime_types.emplace(".pre", "application/vnd.lotus-freelance");
        mime_types.emplace(".nsf", "application/vnd.lotus-notes");
        mime_types.emplace(".org", "application/vnd.lotus-organizer");
        mime_types.emplace(".scm", "application/vnd.lotus-screencam");
        mime_types.emplace(".lwp", "application/vnd.lotus-wordpro");
        mime_types.emplace(".lvp", "audio/vnd.lucent.voice");
        mime_types.emplace(".m3u", "audio/x-mpegurl");
        mime_types.emplace(".m4v", "video/x-m4v");
        mime_types.emplace(".hqx", "application/mac-binhex40");
        mime_types.emplace(".portpkg", "application/vnd.macports.portpkg");
        mime_types.emplace(".mgp", "application/vnd.osgeo.mapguide.package");
        mime_types.emplace(".mrc", "application/marc");
        mime_types.emplace(".mrcx", "application/marcxml+xml");
        mime_types.emplace(".mxf", "application/mxf");
        mime_types.emplace(".nbp", "application/vnd.wolfram.player");
        mime_types.emplace(".ma", "application/mathematica");
        mime_types.emplace(".mathml", "application/mathml+xml");
        mime_types.emplace(".mbox", "application/mbox");
        mime_types.emplace(".mc1", "application/vnd.medcalcdata");
        mime_types.emplace(".mscml", "application/mediaservercontrol+xml");
        mime_types.emplace(".cdkey", "application/vnd.mediastation.cdkey");
        mime_types.emplace(".mwf", "application/vnd.mfer");
        mime_types.emplace(".mfm", "application/vnd.mfmp");
        mime_types.emplace(".msh", "model/mesh");
        mime_types.emplace(".mads", "application/mads+xml");
        mime_types.emplace(".mets", "application/mets+xml");
        mime_types.emplace(".mods", "application/mods+xml");
        mime_types.emplace(".meta4", "application/metalink4+xml");
        mime_types.emplace(".mcd", "application/vnd.mcd");
        mime_types.emplace(".flo", "application/vnd.micrografx.flo");
        mime_types.emplace(".igx", "application/vnd.micrografx.igx");
        mime_types.emplace(".es3", "application/vnd.eszigno3+xml");
        mime_types.emplace(".mdb", "application/x-msaccess");
        mime_types.emplace(".asf", "video/x-ms-asf");
        mime_types.emplace(".exe", "application/x-msdownload");
        mime_types.emplace(".cil", "application/vnd.ms-artgalry");
        mime_types.emplace(".cab", "application/vnd.ms-cab-compressed");
        mime_types.emplace(".ims", "application/vnd.ms-ims");
        mime_types.emplace(".application", "application/x-ms-application");
        mime_types.emplace(".clp", "application/x-msclip");
        mime_types.emplace(".mdi", "image/vnd.ms-modi");
        mime_types.emplace(".eot", "application/vnd.ms-fontobject");
        mime_types.emplace(".xls", "application/vnd.ms-excel");
        mime_types.emplace(".xlam", "application/vnd.ms-excel.addin.macroenabled.12");
        mime_types.emplace(".xlsb", "application/vnd.ms-excel.sheet.binary.macroenabled.12");
        mime_types.emplace(".xltm", "application/vnd.ms-excel.template.macroenabled.12");
        mime_types.emplace(".xlsm", "application/vnd.ms-excel.sheet.macroenabled.12");
        mime_types.emplace(".chm", "application/vnd.ms-htmlhelp");
        mime_types.emplace(".crd", "application/x-mscardfile");
        mime_types.emplace(".lrm", "application/vnd.ms-lrm");
        mime_types.emplace(".mvb", "application/x-msmediaview");
        mime_types.emplace(".mny", "application/x-msmoney");
        mime_types.emplace(".pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation");
        mime_types.emplace(".sldx", "application/vnd.openxmlformats-officedocument.presentationml.slide");
        mime_types.emplace(".ppsx", "application/vnd.openxmlformats-officedocument.presentationml.slideshow");
        mime_types.emplace(".potx", "application/vnd.openxmlformats-officedocument.presentationml.template");
        mime_types.emplace(".xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet");
        mime_types.emplace(".xltx", "application/vnd.openxmlformats-officedocument.spreadsheetml.template");
        mime_types.emplace(".docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document");
        mime_types.emplace(".dotx", "application/vnd.openxmlformats-officedocument.wordprocessingml.template");
        mime_types.emplace(".obd", "application/x-msbinder");
        mime_types.emplace(".thmx", "application/vnd.ms-officetheme");
        mime_types.emplace(".onetoc", "application/onenote");
        mime_types.emplace(".pya", "audio/vnd.ms-playready.media.pya");
        mime_types.emplace(".pyv", "video/vnd.ms-playready.media.pyv");
        mime_types.emplace(".ppt", "application/vnd.ms-powerpoint");
        mime_types.emplace(".ppam", "application/vnd.ms-powerpoint.addin.macroenabled.12");
        mime_types.emplace(".sldm", "application/vnd.ms-powerpoint.slide.macroenabled.12");
        mime_types.emplace(".pptm", "application/vnd.ms-powerpoint.presentation.macroenabled.12");
        mime_types.emplace(".ppsm", "application/vnd.ms-powerpoint.slideshow.macroenabled.12");
        mime_types.emplace(".potm", "application/vnd.ms-powerpoint.template.macroenabled.12");
        mime_types.emplace(".mpp", "application/vnd.ms-project");
        mime_types.emplace(".pub", "application/x-mspublisher");
        mime_types.emplace(".scd", "application/x-msschedule");
        mime_types.emplace(".xap", "application/x-silverlight-app");
        mime_types.emplace(".stl", "application/vnd.ms-pki.stl");
        mime_types.emplace(".cat", "application/vnd.ms-pki.seccat");
        mime_types.emplace(".vsd", "application/vnd.visio");
        mime_types.emplace(".vsdx", "application/vnd.visio2013");
        mime_types.emplace(".wm", "video/x-ms-wm");
        mime_types.emplace(".wma", "audio/x-ms-wma");
        mime_types.emplace(".wax", "audio/x-ms-wax");
        mime_types.emplace(".wmx", "video/x-ms-wmx");
        mime_types.emplace(".wmd", "application/x-ms-wmd");
        mime_types.emplace(".wpl", "application/vnd.ms-wpl");
        mime_types.emplace(".wmz", "application/x-ms-wmz");
        mime_types.emplace(".wmv", "video/x-ms-wmv");
        mime_types.emplace(".wvx", "video/x-ms-wvx");
        mime_types.emplace(".wmf", "application/x-msmetafile");
        mime_types.emplace(".trm", "application/x-msterminal");
        mime_types.emplace(".doc", "application/msword");
        mime_types.emplace(".docm", "application/vnd.ms-word.document.macroenabled.12");
        mime_types.emplace(".dotm", "application/vnd.ms-word.template.macroenabled.12");
        mime_types.emplace(".wri", "application/x-mswrite");
        mime_types.emplace(".wps", "application/vnd.ms-works");
        mime_types.emplace(".xbap", "application/x-ms-xbap");
        mime_types.emplace(".xps", "application/vnd.ms-xpsdocument");
        mime_types.emplace(".mid", "audio/midi");
        mime_types.emplace(".mpy", "application/vnd.ibm.minipay");
        mime_types.emplace(".afp", "application/vnd.ibm.modcap");
        mime_types.emplace(".rms", "application/vnd.jcp.javame.midlet-rms");
        mime_types.emplace(".tmo", "application/vnd.tmobile-livetv");
        mime_types.emplace(".prc", "application/x-mobipocket-ebook");
        mime_types.emplace(".mbk", "application/vnd.mobius.mbk");
        mime_types.emplace(".dis", "application/vnd.mobius.dis");
        mime_types.emplace(".plc", "application/vnd.mobius.plc");
        mime_types.emplace(".mqy", "application/vnd.mobius.mqy");
        mime_types.emplace(".msl", "application/vnd.mobius.msl");
        mime_types.emplace(".txf", "application/vnd.mobius.txf");
        mime_types.emplace(".daf", "application/vnd.mobius.daf");
        mime_types.emplace(".fly", "text/vnd.fly");
        mime_types.emplace(".mpc", "application/vnd.mophun.certificate");
        mime_types.emplace(".mpn", "application/vnd.mophun.application");
        mime_types.emplace(".mj2", "video/mj2");
        mime_types.emplace(".mpga", "audio/mpeg");
        mime_types.emplace(".mxu", "video/vnd.mpegurl");
        mime_types.emplace(".mpeg", "video/mpeg");
        mime_types.emplace(".m21", "application/mp21");
        mime_types.emplace(".mp4a", "audio/mp4");
        mime_types.emplace(".mp4", "video/mp4");
        mime_types.emplace(".mp4", "application/mp4");
        mime_types.emplace(".m3u8", "application/vnd.apple.mpegurl");
        mime_types.emplace(".mus", "application/vnd.musician");
        mime_types.emplace(".msty", "application/vnd.muvee.style");
        mime_types.emplace(".mxml", "application/xv+xml");
        mime_types.emplace(".ngdat", "application/vnd.nokia.n-gage.data");
        mime_types.emplace(".n-gage", "application/vnd.nokia.n-gage.symbian.install");
        mime_types.emplace(".ncx", "application/x-dtbncx+xml");
        mime_types.emplace(".nc", "application/x-netcdf");
        mime_types.emplace(".nlu", "application/vnd.neurolanguage.nlu");
        mime_types.emplace(".dna", "application/vnd.dna");
        mime_types.emplace(".nnd", "application/vnd.noblenet-directory");
        mime_types.emplace(".nns", "application/vnd.noblenet-sealer");
        mime_types.emplace(".nnw", "application/vnd.noblenet-web");
        mime_types.emplace(".rpst", "application/vnd.nokia.radio-preset");
        mime_types.emplace(".rpss", "application/vnd.nokia.radio-presets");
        mime_types.emplace(".n3", "text/n3");
        mime_types.emplace(".edm", "application/vnd.novadigm.edm");
        mime_types.emplace(".edx", "application/vnd.novadigm.edx");
        mime_types.emplace(".ext", "application/vnd.novadigm.ext");
        mime_types.emplace(".gph", "application/vnd.flographit");
        mime_types.emplace(".ecelp4800", "audio/vnd.nuera.ecelp4800");
        mime_types.emplace(".ecelp7470", "audio/vnd.nuera.ecelp7470");
        mime_types.emplace(".ecelp9600", "audio/vnd.nuera.ecelp9600");
        mime_types.emplace(".oda", "application/oda");
        mime_types.emplace(".ogx", "application/ogg");
        mime_types.emplace(".oga", "audio/ogg");
        mime_types.emplace(".ogv", "video/ogg");
        mime_types.emplace(".dd2", "application/vnd.oma.dd2+xml");
        mime_types.emplace(".oth", "application/vnd.oasis.opendocument.text-web");
        mime_types.emplace(".opf", "application/oebps-package+xml");
        mime_types.emplace(".qbo", "application/vnd.intu.qbo");
        mime_types.emplace(".oxt", "application/vnd.openofficeorg.extension");
        mime_types.emplace(".osf", "application/vnd.yamaha.openscoreformat");
        mime_types.emplace(".weba", "audio/webm");
        mime_types.emplace(".webm", "video/webm");
        mime_types.emplace(".odc", "application/vnd.oasis.opendocument.chart");
        mime_types.emplace(".otc", "application/vnd.oasis.opendocument.chart-template");
        mime_types.emplace(".odb", "application/vnd.oasis.opendocument.database");
        mime_types.emplace(".odf", "application/vnd.oasis.opendocument.formula");
        mime_types.emplace(".odft", "application/vnd.oasis.opendocument.formula-template");
        mime_types.emplace(".odg", "application/vnd.oasis.opendocument.graphics");
        mime_types.emplace(".otg", "application/vnd.oasis.opendocument.graphics-template");
        mime_types.emplace(".odi", "application/vnd.oasis.opendocument.image");
        mime_types.emplace(".oti", "application/vnd.oasis.opendocument.image-template");
        mime_types.emplace(".odp", "application/vnd.oasis.opendocument.presentation");
        mime_types.emplace(".otp", "application/vnd.oasis.opendocument.presentation-template");
        mime_types.emplace(".ods", "application/vnd.oasis.opendocument.spreadsheet");
        mime_types.emplace(".ots", "application/vnd.oasis.opendocument.spreadsheet-template");
        mime_types.emplace(".odt", "application/vnd.oasis.opendocument.text");
        mime_types.emplace(".odm", "application/vnd.oasis.opendocument.text-master");
        mime_types.emplace(".ott", "application/vnd.oasis.opendocument.text-template");
        mime_types.emplace(".ktx", "image/ktx");
        mime_types.emplace(".sxc", "application/vnd.sun.xml.calc");
        mime_types.emplace(".stc", "application/vnd.sun.xml.calc.template");
        mime_types.emplace(".sxd", "application/vnd.sun.xml.draw");
        mime_types.emplace(".std", "application/vnd.sun.xml.draw.template");
        mime_types.emplace(".sxi", "application/vnd.sun.xml.impress");
        mime_types.emplace(".sti", "application/vnd.sun.xml.impress.template");
        mime_types.emplace(".sxm", "application/vnd.sun.xml.math");
        mime_types.emplace(".sxw", "application/vnd.sun.xml.writer");
        mime_types.emplace(".sxg", "application/vnd.sun.xml.writer.global");
        mime_types.emplace(".stw", "application/vnd.sun.xml.writer.template");
        mime_types.emplace(".otf", "application/x-font-otf");
        mime_types.emplace(".osfpvg", "application/vnd.yamaha.openscoreformat.osfpvg+xml");
        mime_types.emplace(".dp", "application/vnd.osgi.dp");
        mime_types.emplace(".pdb", "application/vnd.palm");
        mime_types.emplace(".p", "text/x-pascal");
        mime_types.emplace(".paw", "application/vnd.pawaafile");
        mime_types.emplace(".pclxl", "application/vnd.hp-pclxl");
        mime_types.emplace(".efif", "application/vnd.picsel");
        mime_types.emplace(".pcx", "image/x-pcx");
        mime_types.emplace(".psd", "image/vnd.adobe.photoshop");
        mime_types.emplace(".prf", "application/pics-rules");
        mime_types.emplace(".pic", "image/x-pict");
        mime_types.emplace(".chat", "application/x-chat");
        mime_types.emplace(".p10", "application/pkcs10");
        mime_types.emplace(".p12", "application/x-pkcs12");
        mime_types.emplace(".p7m", "application/pkcs7-mime");
        mime_types.emplace(".p7s", "application/pkcs7-signature");
        mime_types.emplace(".p7r", "application/x-pkcs7-certreqresp");
        mime_types.emplace(".p7b", "application/x-pkcs7-certificates");
        mime_types.emplace(".p8", "application/pkcs8");
        mime_types.emplace(".plf", "application/vnd.pocketlearn");
        mime_types.emplace(".pnm", "image/x-portable-anymap");
        mime_types.emplace(".pbm", "image/x-portable-bitmap");
        mime_types.emplace(".pcf", "application/x-font-pcf");
        mime_types.emplace(".pfr", "application/font-tdpfr");
        mime_types.emplace(".pgn", "application/x-chess-pgn");
        mime_types.emplace(".pgm", "image/x-portable-graymap");
        mime_types.emplace(".png", "image/png");
        mime_types.emplace(".png", "image/x-citrix-png");
        mime_types.emplace(".png", "image/x-png");
        mime_types.emplace(".ppm", "image/x-portable-pixmap");
        mime_types.emplace(".pskcxml", "application/pskc+xml");
        mime_types.emplace(".pml", "application/vnd.ctc-posml");
        mime_types.emplace(".ai", "application/postscript");
        mime_types.emplace(".pfa", "application/x-font-type1");
        mime_types.emplace(".pbd", "application/vnd.powerbuilder6");
        mime_types.emplace(".pgp", "application/pgp-encrypted");
        mime_types.emplace(".pgp", "application/pgp-signature");
        mime_types.emplace(".box", "application/vnd.previewsystems.box");
        mime_types.emplace(".ptid", "application/vnd.pvi.ptid1");
        mime_types.emplace(".pls", "application/pls+xml");
        mime_types.emplace(".str", "application/vnd.pg.format");
        mime_types.emplace(".ei6", "application/vnd.pg.osasli");
        mime_types.emplace(".dsc", "text/prs.lines.tag");
        mime_types.emplace(".psf", "application/x-font-linux-psf");
        mime_types.emplace(".qps", "application/vnd.publishare-delta-tree");
        mime_types.emplace(".wg", "application/vnd.pmi.widget");
        mime_types.emplace(".qxd", "application/vnd.quark.quarkxpress");
        mime_types.emplace(".esf", "application/vnd.epson.esf");
        mime_types.emplace(".msf", "application/vnd.epson.msf");
        mime_types.emplace(".ssf", "application/vnd.epson.ssf");
        mime_types.emplace(".qam", "application/vnd.epson.quickanime");
        mime_types.emplace(".qfx", "application/vnd.intu.qfx");
        mime_types.emplace(".qt", "video/quicktime");
        mime_types.emplace(".rar", "application/x-rar-compressed");
        mime_types.emplace(".ram", "audio/x-pn-realaudio");
        mime_types.emplace(".rmp", "audio/x-pn-realaudio-plugin");
        mime_types.emplace(".rsd", "application/rsd+xml");
        mime_types.emplace(".rm", "application/vnd.rn-realmedia");
        mime_types.emplace(".bed", "application/vnd.realvnc.bed");
        mime_types.emplace(".mxl", "application/vnd.recordare.musicxml");
        mime_types.emplace(".musicxml", "application/vnd.recordare.musicxml+xml");
        mime_types.emplace(".rnc", "application/relax-ng-compact-syntax");
        mime_types.emplace(".rdz", "application/vnd.data-vision.rdz");
        mime_types.emplace(".rdf", "application/rdf+xml");
        mime_types.emplace(".rp9", "application/vnd.cloanto.rp9");
        mime_types.emplace(".jisp", "application/vnd.jisp");
        mime_types.emplace(".rtf", "application/rtf");
        mime_types.emplace(".rtx", "text/richtext");
        mime_types.emplace(".link66", "application/vnd.route66.link66+xml");
        mime_types.emplace(".rss, .xml", "application/rss+xml");
        mime_types.emplace(".shf", "application/shf+xml");
        mime_types.emplace(".st", "application/vnd.sailingtracker.track");
        mime_types.emplace(".svg", "image/svg+xml");
        mime_types.emplace(".sus", "application/vnd.sus-calendar");
        mime_types.emplace(".sru", "application/sru+xml");
        mime_types.emplace(".setpay", "application/set-payment-initiation");
        mime_types.emplace(".setreg", "application/set-registration-initiation");
        mime_types.emplace(".sema", "application/vnd.sema");
        mime_types.emplace(".semd", "application/vnd.semd");
        mime_types.emplace(".semf", "application/vnd.semf");
        mime_types.emplace(".see", "application/vnd.seemail");
        mime_types.emplace(".snf", "application/x-font-snf");
        mime_types.emplace(".spq", "application/scvp-vp-request");
        mime_types.emplace(".spp", "application/scvp-vp-response");
        mime_types.emplace(".scq", "application/scvp-cv-request");
        mime_types.emplace(".scs", "application/scvp-cv-response");
        mime_types.emplace(".sdp", "application/sdp");
        mime_types.emplace(".etx", "text/x-setext");
        mime_types.emplace(".movie", "video/x-sgi-movie");
        mime_types.emplace(".ifm", "application/vnd.shana.informed.formdata");
        mime_types.emplace(".itp", "application/vnd.shana.informed.formtemplate");
        mime_types.emplace(".iif", "application/vnd.shana.informed.interchange");
        mime_types.emplace(".ipk", "application/vnd.shana.informed.package");
        mime_types.emplace(".tfi", "application/thraud+xml");
        mime_types.emplace(".shar", "application/x-shar");
        mime_types.emplace(".rgb", "image/x-rgb");
        mime_types.emplace(".slt", "application/vnd.epson.salt");
        mime_types.emplace(".aso", "application/vnd.accpac.simply.aso");
        mime_types.emplace(".imp", "application/vnd.accpac.simply.imp");
        mime_types.emplace(".twd", "application/vnd.simtech-mindmapper");
        mime_types.emplace(".csp", "application/vnd.commonspace");
        mime_types.emplace(".saf", "application/vnd.yamaha.smaf-audio");
        mime_types.emplace(".mmf", "application/vnd.smaf");
        mime_types.emplace(".spf", "application/vnd.yamaha.smaf-phrase");
        mime_types.emplace(".teacher", "application/vnd.smart.teacher");
        mime_types.emplace(".svd", "application/vnd.svd");
        mime_types.emplace(".rq", "application/sparql-query");
        mime_types.emplace(".srx", "application/sparql-results+xml");
        mime_types.emplace(".gram", "application/srgs");
        mime_types.emplace(".grxml", "application/srgs+xml");
        mime_types.emplace(".ssml", "application/ssml+xml");
        mime_types.emplace(".skp", "application/vnd.koan");
        mime_types.emplace(".sgml", "text/sgml");
        mime_types.emplace(".sdc", "application/vnd.stardivision.calc");
        mime_types.emplace(".sda", "application/vnd.stardivision.draw");
        mime_types.emplace(".sdd", "application/vnd.stardivision.impress");
        mime_types.emplace(".smf", "application/vnd.stardivision.math");
        mime_types.emplace(".sdw", "application/vnd.stardivision.writer");
        mime_types.emplace(".sgl", "application/vnd.stardivision.writer-global");
        mime_types.emplace(".sm", "application/vnd.stepmania.stepchart");
        mime_types.emplace(".sit", "application/x-stuffit");
        mime_types.emplace(".sitx", "application/x-stuffitx");
        mime_types.emplace(".sdkm", "application/vnd.solent.sdkm+xml");
        mime_types.emplace(".xo", "application/vnd.olpc-sugar");
        mime_types.emplace(".au", "audio/basic");
        mime_types.emplace(".wqd", "application/vnd.wqd");
        mime_types.emplace(".sis", "application/vnd.symbian.install");
        mime_types.emplace(".smi", "application/smil+xml");
        mime_types.emplace(".xsm", "application/vnd.syncml+xml");
        mime_types.emplace(".bdm", "application/vnd.syncml.dm+wbxml");
        mime_types.emplace(".xdm", "application/vnd.syncml.dm+xml");
        mime_types.emplace(".sv4cpio", "application/x-sv4cpio");
        mime_types.emplace(".sv4crc", "application/x-sv4crc");
        mime_types.emplace(".sbml", "application/sbml+xml");
        mime_types.emplace(".tsv", "text/tab-separated-values");
        mime_types.emplace(".tiff", "image/tiff");
        mime_types.emplace(".tao", "application/vnd.tao.intent-module-archive");
        mime_types.emplace(".tar", "application/x-tar");
        mime_types.emplace(".tcl", "application/x-tcl");
        mime_types.emplace(".tex", "application/x-tex");
        mime_types.emplace(".tfm", "application/x-tex-tfm");
        mime_types.emplace(".tei", "application/tei+xml");
        mime_types.emplace(".txt", "text/plain");
        mime_types.emplace(".dxp", "application/vnd.spotfire.dxp");
        mime_types.emplace(".sfs", "application/vnd.spotfire.sfs");
        mime_types.emplace(".tsd", "application/timestamped-data");
        mime_types.emplace(".tpt", "application/vnd.trid.tpt");
        mime_types.emplace(".mxs", "application/vnd.triscape.mxs");
        mime_types.emplace(".t", "text/troff");
        mime_types.emplace(".tra", "application/vnd.trueapp");
        mime_types.emplace(".ttf", "application/x-font-ttf");
        mime_types.emplace(".ttl", "text/turtle");
        mime_types.emplace(".umj", "application/vnd.umajin");
        mime_types.emplace(".uoml", "application/vnd.uoml+xml");
        mime_types.emplace(".unityweb", "application/vnd.unity");
        mime_types.emplace(".ufd", "application/vnd.ufdl");
        mime_types.emplace(".uri", "text/uri-list");
        mime_types.emplace(".utz", "application/vnd.uiq.theme");
        mime_types.emplace(".ustar", "application/x-ustar");
        mime_types.emplace(".uu", "text/x-uuencode");
        mime_types.emplace(".vcs", "text/x-vcalendar");
        mime_types.emplace(".vcf", "text/x-vcard");
        mime_types.emplace(".vcd", "application/x-cdlink");
        mime_types.emplace(".vsf", "application/vnd.vsf");
        mime_types.emplace(".wrl", "model/vrml");
        mime_types.emplace(".vcx", "application/vnd.vcx");
        mime_types.emplace(".mts", "model/vnd.mts");
        mime_types.emplace(".vtu", "model/vnd.vtu");
        mime_types.emplace(".vis", "application/vnd.visionary");
        mime_types.emplace(".viv", "video/vnd.vivo");
        mime_types.emplace(".ccxml", "application/ccxml+xml,");
        mime_types.emplace(".vxml", "application/voicexml+xml");
        mime_types.emplace(".src", "application/x-wais-source");
        mime_types.emplace(".wbxml", "application/vnd.wap.wbxml");
        mime_types.emplace(".wbmp", "image/vnd.wap.wbmp");
        mime_types.emplace(".wav", "audio/x-wav");
        mime_types.emplace(".davmount", "application/davmount+xml");
        mime_types.emplace(".woff", "application/x-font-woff");
        mime_types.emplace(".wspolicy", "application/wspolicy+xml");
        mime_types.emplace(".webp", "image/webp");
        mime_types.emplace(".wtb", "application/vnd.webturbo");
        mime_types.emplace(".wgt", "application/widget");
        mime_types.emplace(".hlp", "application/winhlp");
        mime_types.emplace(".wml", "text/vnd.wap.wml");
        mime_types.emplace(".wmls", "text/vnd.wap.wmlscript");
        mime_types.emplace(".wmlsc", "application/vnd.wap.wmlscriptc");
        mime_types.emplace(".wpd", "application/vnd.wordperfect");
        mime_types.emplace(".stf", "application/vnd.wt.stf");
        mime_types.emplace(".wsdl", "application/wsdl+xml");
        mime_types.emplace(".xbm", "image/x-xbitmap");
        mime_types.emplace(".xpm", "image/x-xpixmap");
        mime_types.emplace(".xwd", "image/x-xwindowdump");
        mime_types.emplace(".der", "application/x-x509-ca-cert");
        mime_types.emplace(".fig", "application/x-xfig");
        mime_types.emplace(".xhtml", "application/xhtml+xml");
        mime_types.emplace(".xml", "application/xml");
        mime_types.emplace(".xdf", "application/xcap-diff+xml");
        mime_types.emplace(".xenc", "application/xenc+xml");
        mime_types.emplace(".xer", "application/patch-ops-error+xml");
        mime_types.emplace(".rl", "application/resource-lists+xml");
        mime_types.emplace(".rs", "application/rls-services+xml");
        mime_types.emplace(".rld", "application/resource-lists-diff+xml");
        mime_types.emplace(".xslt", "application/xslt+xml");
        mime_types.emplace(".xop", "application/xop+xml");
        mime_types.emplace(".xpi", "application/x-xpinstall");
        mime_types.emplace(".xspf", "application/xspf+xml");
        mime_types.emplace(".xul", "application/vnd.mozilla.xul+xml");
        mime_types.emplace(".xyz", "chemical/x-xyz");
        mime_types.emplace(".yaml", "text/yaml");
        mime_types.emplace(".yang", "application/yang");
        mime_types.emplace(".yin", "application/yin+xml");
        mime_types.emplace(".zir", "application/vnd.zul");
        mime_types.emplace(".zip", "application/zip");
        mime_types.emplace(".zmm", "application/vnd.handheld-entertainment+xml");
        mime_types.emplace(".zaz", "application/vnd.zzazz.deck+xml");
    }

    const std::string &Http::get_mimetype(const std::string &filename)
    {
        //Try to isolate extention from filename variable
        std::string extention;
        auto dotPos = filename.find_last_of('.');
        if(dotPos != std::string::npos)
            extention = filename.substr(dotPos, filename.size() - dotPos);
        else
            extention = "." + filename;

        //Find it
        auto iter = mime_types.find(extention);
        if(iter == mime_types.end())
            return mime_types[".bin"];
        return iter->second;
    }
}