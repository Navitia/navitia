#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE test_adapted
#include "naviMake/data.h"
#include <boost/test/unit_test.hpp>
#include <string>
#include "config.h"
#include "naviMake/build_helper.h"
#include "naviMake/adapted.h"
#include "naviMake/types.h"
#include "boost/date_time/gregorian_calendar.hpp"

namespace pt = boost::posix_time;

using namespace navimake;
using namespace navimake::types;

namespace nt = navitia::type;
namespace bg = boost::gregorian;
namespace bt = boost::date_time;
namespace navimake{ namespace types{

    std::ostream& operator<<(std::ostream& cout, const ValidityPattern& vp){
        cout << "[" << vp.beginning_date << " / " << vp.days << "]";
        return cout;
    }
}}
//						vendredi 01 mars                                                                    dimanche 03 mars
//                              |	 			            |	       					|      						|
//daily_start_hour              |			              	|---------------------------|---------------------------|
//                              |	       					|///////////////////////////|///////////////////////////|
//application_period            |---------------------------|///////////////////////////|///////////////////////////|
//                              |///////////////////////////|///////////////////////////|///////////////////////////|
//application_period            |///////////////////////////|///////////////////////////|---------------------------|
//                              |///////////////////////////|///////////////////////////|			              	|
//daily_end_hour                |---------------------------|---------------------------|                           |
//                              |					      	|			              	|	    					|


//le vj1 circule tous les jours du vendredi 01 mars au jeudi 07 mars
//le vj2 ne circule pas le lundi 04 mars et le mardi 05 mars

//le message s'applique sur le vj1 le vendredi 01 mars
BOOST_AUTO_TEST_CASE(impact_vj_0){
    navimake::builder b("20130301T1739");
    bg::date end_date = bg::date_from_iso_string("20130308T1739");
    b.generate_dummy_basis();
    VehicleJourney* vj = b.vj("A", "", "", true, "vj1")("stop1", 8000,8050)("stop2", 8200,8250).vj;
    //construction du validityPattern du vj1: 1111111
    std::bitset<7> validedays;
    validedays[bt::Sunday] = true;
    validedays[bt::Monday] = true;
    validedays[bt::Tuesday] = true;
    validedays[bt::Wednesday] = true;
    validedays[bt::Thursday] = true;
    validedays[bt::Friday] = true;
    validedays[bt::Saturday] = true;
    vj->validity_pattern->add(vj->validity_pattern->beginning_date, end_date, validedays);
    vj->adapted_validity_pattern->add(vj->adapted_validity_pattern->beginning_date, end_date, validedays);

    vj = b.vj("A", "", "", true, "vj2")("stop1", 9000,9050)("stop2", 9200,9250).vj;
    //construction du validityPattern du vj2: 1110011 (1er mars est un vendredi)
    validedays[bt::Sunday] = true;
    validedays[bt::Monday] = false;
    validedays[bt::Tuesday] = false;
    validedays[bt::Wednesday] = true;
    validedays[bt::Thursday] = true;
    validedays[bt::Friday] = true;
    validedays[bt::Saturday] = true;
    vj->validity_pattern->add(vj->validity_pattern->beginning_date, end_date, validedays);
    vj->adapted_validity_pattern->add(vj->adapted_validity_pattern->beginning_date, end_date, validedays);

    b.data.normalize_uri();


    std::map<std::string, std::vector<nt::Message>> messages;
    nt::Message m;
    m.object_type = nt::Type_e::VehicleJourney;
    m.object_uri = "vehicle_journey:vj1";
    m.application_period = pt::time_period(pt::time_from_string("2013-03-01 00:00:00"), pt::time_from_string("2013-03-03 23:59:00"));
    m.active_days = nt::Ven;
    m.application_daily_start_hour = pt::duration_from_string("00:00");
    m.application_daily_end_hour = pt::duration_from_string("23:59");

    messages[m.object_uri].push_back(m);

    AtAdaptedLoader loader;
    loader.apply(messages, b.data);

    vj = b.data.vehicle_journeys[0];
    BOOST_CHECK_EQUAL(vj->uri,  "vehicle_journey:vj1");

//    BOOST_CHECK_EQUAL(*vj->validity_pattern,  ValidityPattern(b.begin, "1111111"));
    bg::date testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

//    BOOST_CHECK_EQUAL(*vj->adapted_validity_pattern,  ValidityPattern(b.begin, "1111110"));
    //le message s'applique sur le vj1 le vendredi 01 mars
    testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    vj = b.data.vehicle_journeys[1];

    BOOST_CHECK_EQUAL(vj->uri,  "vehicle_journey:vj2");
//    BOOST_CHECK_EQUAL(*vj->validity_pattern,  ValidityPattern(b.begin, "1110011"));
    testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    BOOST_CHECK_EQUAL(*vj->adapted_validity_pattern, *vj->validity_pattern);
    BOOST_CHECK_EQUAL(b.data.vehicle_journeys.size(), 2);
}

//le vj1 circule tous les jours du vendredi 01 mars au jeudi 07 mars
//le vj2 ne circule pas le lundi 04 mars et le mardi 05 mars

//le message s'applique sur le vj1 le vendredi 01 mars, samedi 02 mars et le dimanche 03 mars.
BOOST_AUTO_TEST_CASE(impact_vj_1){
    navimake::builder b("20130301T1739");
    bg::date end_date = bg::date_from_iso_string("20130308T1739");
    b.generate_dummy_basis();
    VehicleJourney* vj = b.vj("A", "", "", true, "vj1")("stop1", 8000,8050)("stop2", 8200,8250).vj;
    //construction du validityPattern du vj1: 1111111
    std::bitset<7> validedays;
    validedays[bt::Sunday] = true;
    validedays[bt::Monday] = true;
    validedays[bt::Tuesday] = true;
    validedays[bt::Wednesday] = true;
    validedays[bt::Thursday] = true;
    validedays[bt::Friday] = true;
    validedays[bt::Saturday] = true;
    vj->validity_pattern->add(vj->validity_pattern->beginning_date, end_date, validedays);
    vj->adapted_validity_pattern->add(vj->adapted_validity_pattern->beginning_date, end_date, validedays);

    vj = b.vj("A", "", "", true, "vj2")("stop1", 9000,9050)("stop2", 9200,9250).vj;
    //construction du validityPattern du vj2: 1110011 (1er mars est un vendredi)
    validedays[bt::Sunday] = true;
    validedays[bt::Monday] = false;
    validedays[bt::Tuesday] = false;
    validedays[bt::Wednesday] = true;
    validedays[bt::Thursday] = true;
    validedays[bt::Friday] = true;
    validedays[bt::Saturday] = true;
    vj->validity_pattern->add(vj->validity_pattern->beginning_date, end_date, validedays);
    vj->adapted_validity_pattern->add(vj->adapted_validity_pattern->beginning_date, end_date, validedays);

    b.data.normalize_uri();


    std::map<std::string, std::vector<nt::Message>> messages;
    nt::Message m;
    m.object_type = nt::Type_e::VehicleJourney;
    m.object_uri = "vehicle_journey:vj1";
    m.application_period = pt::time_period(pt::time_from_string("2013-03-01 00:00:00"), pt::time_from_string("2013-03-03 23:59:00"));
    m.active_days = nt::Lun | nt::Mar | nt::Mer | nt::Jeu | nt::Ven | nt::Sam | nt::Dim;
    m.application_daily_start_hour = pt::duration_from_string("00:00");
    m.application_daily_end_hour = pt::duration_from_string("23:59");

    messages[m.object_uri].push_back(m);

    AtAdaptedLoader loader;
    loader.apply(messages, b.data);

    vj = b.data.vehicle_journeys[0];

    BOOST_CHECK_EQUAL(vj->uri,  "vehicle_journey:vj1");
//    BOOST_CHECK_EQUAL(*vj->validity_pattern,  ValidityPattern(b.begin, "1111111"));
    bg::date testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);


//    BOOST_CHECK_EQUAL(*vj->adapted_validity_pattern,  ValidityPattern(b.begin, "1111000"));
    testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);


    vj = b.data.vehicle_journeys[1];

    BOOST_CHECK_EQUAL(vj->uri,  "vehicle_journey:vj2");
//    BOOST_CHECK_EQUAL(*vj->validity_pattern,  ValidityPattern(b.begin, "1110011"));
    testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    BOOST_CHECK_EQUAL(*vj->adapted_validity_pattern, *vj->validity_pattern);

    BOOST_CHECK_EQUAL(b.data.vehicle_journeys.size(), 2);
}

//le vj1 circule tous les jours du vendredi 01 mars au jeudi 07 mars
//le vj2 ne circule pas le lundi 04 mars et le mardi 05 mars

//le message s'applique sur le vj1 le samedi 02 mars et le dimanche 03 mars. (pas le vendredi 01 car heures non valides)
//il ne s'applique pas sur le vj2 car heures non valides
BOOST_AUTO_TEST_CASE(impact_vj_2){
    navimake::builder b("20130301T1739");
    bg::date end_date = bg::date_from_iso_string("20130308T1739");
    b.generate_dummy_basis();
    VehicleJourney* vj = b.vj("A", "", "", true, "vj1")("stop1", "8:50","9:00")("stop2", "11:00", "11:02").vj;
    //construction du validityPattern du vj1: 1111111
    std::bitset<7> validedays;
    validedays[bt::Sunday] = true;
    validedays[bt::Monday] = true;
    validedays[bt::Tuesday] = true;
    validedays[bt::Wednesday] = true;
    validedays[bt::Thursday] = true;
    validedays[bt::Friday] = true;
    validedays[bt::Saturday] = true;
    vj->validity_pattern->add(vj->validity_pattern->beginning_date, end_date, validedays);
    vj->adapted_validity_pattern->add(vj->adapted_validity_pattern->beginning_date, end_date, validedays);

    vj = b.vj("A", "", "", true, "vj2")("stop1", "10:50","11:00")("stop2","13:00","13:02").vj;
    //construction du validityPattern du vj2: 1110011 (1er mars est un vendredi)
    validedays[bt::Sunday] = true;
    validedays[bt::Monday] = false;
    validedays[bt::Tuesday] = false;
    validedays[bt::Wednesday] = true;
    validedays[bt::Thursday] = true;
    validedays[bt::Friday] = true;
    validedays[bt::Saturday] = true;
    vj->validity_pattern->add(vj->validity_pattern->beginning_date, end_date, validedays);
    vj->adapted_validity_pattern->add(vj->adapted_validity_pattern->beginning_date, end_date, validedays);

    b.data.normalize_uri();


    std::map<std::string, std::vector<nt::Message>> messages;
    nt::Message m;
    m.object_type = nt::Type_e::VehicleJourney;
    m.object_uri = "vehicle_journey:vj1";
    m.application_period = pt::time_period(pt::time_from_string("2013-03-01 10:00:00"), pt::time_from_string("2013-03-03 23:59:00"));
    m.active_days = nt::Lun | nt::Mar | nt::Mer | nt::Jeu | nt::Ven | nt::Sam | nt::Dim;
    m.application_daily_start_hour = pt::duration_from_string("00:00");
    m.application_daily_end_hour = pt::duration_from_string("09:30");

    messages[m.object_uri].push_back(m);


    m.object_uri = "vehicle_journey:vj2";
    messages[m.object_uri].push_back(m);

    AtAdaptedLoader loader;
    loader.apply(messages, b.data);

    vj = b.data.vehicle_journeys[0];

    BOOST_CHECK_EQUAL(vj->uri,  "vehicle_journey:vj1");
//    BOOST_CHECK_EQUAL(*vj->validity_pattern,  ValidityPattern(b.begin, "1111111"));
    bg::date testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);


//    BOOST_CHECK_EQUAL(*vj->adapted_validity_pattern,  ValidityPattern(b.begin, "1111001"));
    testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    vj = b.data.vehicle_journeys[1];

    BOOST_CHECK_EQUAL(vj->uri,  "vehicle_journey:vj2");
//    BOOST_CHECK_EQUAL(*vj->validity_pattern,  ValidityPattern(b.begin, "1110011"));
    testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

//    BOOST_CHECK_EQUAL(*vj->adapted_validity_pattern,  ValidityPattern(b.begin, "1110011"));
    testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    BOOST_CHECK_EQUAL(b.data.vehicle_journeys.size(), 2);

}
//vj1 et vj2 ne circule pas le 01, 02 et 03 mars
//vj3 pas impactée
BOOST_AUTO_TEST_CASE(impact_line_0){
    navimake::builder b("201303011T1739");
    bg::date end_date = bg::date_from_iso_string("20130308T1739");
    b.generate_dummy_basis();

    VehicleJourney* vj = b.vj("A", "1111111", "", true, "vj1")("stop1", "8:50","9:00")("stop2", "11:00", "11:02").vj;
    //construction du validityPattern du vj1: 1111111
    std::bitset<7> validedays;
    validedays[bt::Sunday] = true;
    validedays[bt::Monday] = true;
    validedays[bt::Tuesday] = true;
    validedays[bt::Wednesday] = true;
    validedays[bt::Thursday] = true;
    validedays[bt::Friday] = true;
    validedays[bt::Saturday] = true;
    vj->validity_pattern->add(vj->validity_pattern->beginning_date, end_date, validedays);
    vj->adapted_validity_pattern->add(vj->adapted_validity_pattern->beginning_date, end_date, validedays);

    vj = b.vj("A", "1110011", "", true, "vj2")("stop1", "10:50","11:00")("stop2","13:00","13:02").vj;
    //construction du validityPattern du vj2: 1110011
    validedays[bt::Sunday] = true;
    validedays[bt::Monday] = false;
    validedays[bt::Tuesday] = false;
    validedays[bt::Wednesday] = true;
    validedays[bt::Thursday] = true;
    validedays[bt::Friday] = true;
    validedays[bt::Saturday] = true;
    vj->validity_pattern->add(vj->validity_pattern->beginning_date, end_date, validedays);
    vj->adapted_validity_pattern->add(vj->adapted_validity_pattern->beginning_date, end_date, validedays);

    vj = b.vj("B", "1111111", "", true, "vj3")("stop1", "8:50","9:00")("stop2", "11:00", "11:02").vj;
    //construction du validityPattern du vj3: 1111111
    validedays[bt::Sunday] = true;
    validedays[bt::Monday] = true;
    validedays[bt::Tuesday] = true;
    validedays[bt::Wednesday] = true;
    validedays[bt::Thursday] = true;
    validedays[bt::Friday] = true;
    validedays[bt::Saturday] = true;
    vj->validity_pattern->add(vj->validity_pattern->beginning_date, end_date, validedays);
    vj->adapted_validity_pattern->add(vj->adapted_validity_pattern->beginning_date, end_date, validedays);

    b.data.normalize_uri();


    std::map<std::string, std::vector<nt::Message>> messages;
    nt::Message m;
    m.object_type = nt::Type_e::Line;
    m.object_uri = "line:A";
    m.application_period = pt::time_period(pt::time_from_string("2013-03-01 00:00:00"), pt::time_from_string("2013-03-03 23:59:00"));
    m.active_days = nt::Lun | nt::Mar | nt::Mer | nt::Jeu | nt::Ven | nt::Sam | nt::Dim;
    m.application_daily_start_hour = pt::duration_from_string("00:00");
    m.application_daily_end_hour = pt::duration_from_string("23:59");

    messages[m.object_uri].push_back(m);

    AtAdaptedLoader loader;
    loader.apply(messages, b.data);

    vj = b.data.vehicle_journeys[0];

    BOOST_CHECK_EQUAL(vj->uri,  "vehicle_journey:vj1");
//    BOOST_CHECK_EQUAL(*vj->validity_pattern,  ValidityPattern(b.begin, "1111111"));
    bg::date testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

//    BOOST_CHECK_EQUAL(*vj->adapted_validity_pattern,  ValidityPattern(b.begin, "1111000"));
    testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    vj = b.data.vehicle_journeys[1];

    BOOST_CHECK_EQUAL(vj->uri,  "vehicle_journey:vj2");
//    BOOST_CHECK_EQUAL(*vj->validity_pattern,  ValidityPattern(b.begin, "1110011"));
    testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

//    BOOST_CHECK_EQUAL(*vj->adapted_validity_pattern,  ValidityPattern(b.begin, "1110000"));
    testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    vj = b.data.vehicle_journeys[2];

    BOOST_CHECK_EQUAL(vj->uri,  "vehicle_journey:vj3");
//    BOOST_CHECK_EQUAL(*vj->validity_pattern,  ValidityPattern(b.begin, "1111111"));
    testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    BOOST_CHECK_EQUAL(*vj->adapted_validity_pattern, *vj->validity_pattern);

    BOOST_CHECK_EQUAL(b.data.vehicle_journeys.size(), 3);
}
//vj1 et vj2 concerné par le message
//vj1 impacté le vendredi 01
BOOST_AUTO_TEST_CASE(impact_line_1){
    navimake::builder b("201303011T1739");
    bg::date end_date = bg::date_from_iso_string("20130308T1739");
    b.generate_dummy_basis();

    VehicleJourney* vj = b.vj("A", "1111111", "", true, "vj1")("stop1", "8:50","9:00")("stop2", "11:00", "11:02").vj;
    //construction du validityPattern du vj1: 1111111
    std::bitset<7> validedays;
    validedays[bt::Sunday] = true;
    validedays[bt::Monday] = true;
    validedays[bt::Tuesday] = true;
    validedays[bt::Wednesday] = true;
    validedays[bt::Thursday] = true;
    validedays[bt::Friday] = true;
    validedays[bt::Saturday] = true;
    vj->validity_pattern->add(vj->validity_pattern->beginning_date, end_date, validedays);
    vj->adapted_validity_pattern->add(vj->adapted_validity_pattern->beginning_date, end_date, validedays);

    vj = b.vj("A", "1110011", "", true, "vj2")("stop1", "10:50","11:00")("stop2","13:00","13:02").vj;
    //construction du validityPattern du vj2: 1110011
    validedays[bt::Sunday] = true;
    validedays[bt::Monday] = false;
    validedays[bt::Tuesday] = false;
    validedays[bt::Wednesday] = true;
    validedays[bt::Thursday] = true;
    validedays[bt::Friday] = true;
    validedays[bt::Saturday] = true;
    vj->validity_pattern->add(vj->validity_pattern->beginning_date, end_date, validedays);
    vj->adapted_validity_pattern->add(vj->adapted_validity_pattern->beginning_date, end_date, validedays);

    vj = b.vj("B", "1111111", "", true, "vj3")("stop1", "10:50","11:00")("stop2","13:00","13:02").vj;
    //construction du validityPattern du vj3: 1111111
    validedays[bt::Sunday] = true;
    validedays[bt::Monday] = true;
    validedays[bt::Tuesday] = true;
    validedays[bt::Wednesday] = true;
    validedays[bt::Thursday] = true;
    validedays[bt::Friday] = true;
    validedays[bt::Saturday] = true;
    vj->validity_pattern->add(vj->validity_pattern->beginning_date, end_date, validedays);

    b.data.normalize_uri();


    std::map<std::string, std::vector<nt::Message>> messages;
    nt::Message m;
    m.object_type = nt::Type_e::Line;
    m.object_uri = "line:A";
    m.application_period = pt::time_period(pt::time_from_string("2013-03-01 00:00:00"), pt::time_from_string("2013-03-03 23:59:00"));
    m.active_days = nt::Ven;
    m.application_daily_start_hour = pt::duration_from_string("08:30");
    m.application_daily_end_hour = pt::duration_from_string("09:30");

    messages[m.object_uri].push_back(m);

    AtAdaptedLoader loader;
    loader.apply(messages, b.data);

    vj = b.data.vehicle_journeys[0];

    BOOST_CHECK_EQUAL(vj->uri,  "vehicle_journey:vj1");
//    BOOST_CHECK_EQUAL(*vj->validity_pattern,  ValidityPattern(b.begin, "1111111"));
    bg::date testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

//    BOOST_CHECK_EQUAL(*vj->adapted_validity_pattern,  ValidityPattern(b.begin, "1111110"));
    testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    vj = b.data.vehicle_journeys[1];

    BOOST_CHECK_EQUAL(vj->uri,  "vehicle_journey:vj2");
//    BOOST_CHECK_EQUAL(*vj->validity_pattern,  ValidityPattern(b.begin, "1110011"));
    bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

//    BOOST_CHECK_EQUAL(*vj->adapted_validity_pattern,  ValidityPattern(b.begin, "1110011"));
    bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    vj = b.data.vehicle_journeys[2];

    BOOST_CHECK_EQUAL(vj->uri,  "vehicle_journey:vj3");
    BOOST_CHECK_EQUAL(*vj->validity_pattern,  ValidityPattern(b.begin, "1111111"));

    BOOST_CHECK_EQUAL(*vj->adapted_validity_pattern, *vj->validity_pattern);

    BOOST_CHECK_EQUAL(b.data.vehicle_journeys.size(), 3);
}

BOOST_AUTO_TEST_CASE(impact_network_0){
    navimake::builder b("201303011T1739");
    bg::date end_date = bg::date_from_iso_string("20130308T1739");
    b.generate_dummy_basis();
    VehicleJourney* vj = b.vj("A", "A", "1111111", "", true, "vj1")("stop1", "8:50","9:00")("stop2", "11:00", "11:02").vj;
    //construction du validityPattern du vj1: 1111111
    std::bitset<7> validedays;
    validedays[bt::Sunday] = true;
    validedays[bt::Monday] = true;
    validedays[bt::Tuesday] = true;
    validedays[bt::Wednesday] = true;
    validedays[bt::Thursday] = true;
    validedays[bt::Friday] = true;
    validedays[bt::Saturday] = true;
    vj->validity_pattern->add(vj->validity_pattern->beginning_date, end_date, validedays);
    vj->adapted_validity_pattern->add(vj->adapted_validity_pattern->beginning_date, end_date, validedays);

    vj = b.vj("A", "A", "1110011", "", true, "vj2")("stop1", "10:50","11:00")("stop2","13:00","13:02").vj;
    //construction du validityPattern du vj2: 1110011
    validedays[bt::Sunday] = true;
    validedays[bt::Monday] = false;
    validedays[bt::Tuesday] = false;
    validedays[bt::Wednesday] = true;
    validedays[bt::Thursday] = true;
    validedays[bt::Friday] = true;
    validedays[bt::Saturday] = true;
    vj->validity_pattern->add(vj->validity_pattern->beginning_date, end_date, validedays);
    vj->adapted_validity_pattern->add(vj->adapted_validity_pattern->beginning_date, end_date, validedays);

    vj = b.vj("B", "B", "1111111", "", true, "vj3")("stop1", "10:50","11:00")("stop2","13:00","13:02").vj;
    //construction du validityPattern du vj3: 1111111
    validedays[bt::Sunday] = true;
    validedays[bt::Monday] = true;
    validedays[bt::Tuesday] = true;
    validedays[bt::Wednesday] = true;
    validedays[bt::Thursday] = true;
    validedays[bt::Friday] = true;
    validedays[bt::Saturday] = true;
    vj->validity_pattern->add(vj->validity_pattern->beginning_date, end_date, validedays);
    vj->adapted_validity_pattern->add(vj->adapted_validity_pattern->beginning_date, end_date, validedays);

    b.data.normalize_uri();


    std::map<std::string, std::vector<nt::Message>> messages;
    nt::Message m;
    m.object_type = nt::Type_e::Network;
    m.object_uri = "network:A";
    m.application_period = pt::time_period(pt::time_from_string("2013-03-01 00:00:00"), pt::time_from_string("2013-03-03 23:59:00"));
    m.active_days = nt::Lun | nt::Mar | nt::Mer | nt::Jeu | nt::Ven | nt::Sam | nt::Dim;
    m.application_daily_start_hour = pt::duration_from_string("00:00");
    m.application_daily_end_hour = pt::duration_from_string("23:59");

    messages[m.object_uri].push_back(m);

    AtAdaptedLoader loader;
    loader.apply(messages, b.data);

    vj = b.data.vehicle_journeys[0];

    BOOST_CHECK_EQUAL(vj->uri,  "vehicle_journey:vj1");
//    BOOST_CHECK_EQUAL(*vj->validity_pattern,  ValidityPattern(b.begin, "1111111"));
    bg::date testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

//    BOOST_CHECK_EQUAL(*vj->adapted_validity_pattern,  ValidityPattern(b.begin, "1111000"));
    testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    vj = b.data.vehicle_journeys[1];

    BOOST_CHECK_EQUAL(vj->uri,  "vehicle_journey:vj2");
//    BOOST_CHECK_EQUAL(*vj->validity_pattern,  ValidityPattern(b.begin, "1110011"));
    bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

//    BOOST_CHECK_EQUAL(*vj->adapted_validity_pattern,  ValidityPattern(b.begin, "1110000"));
    testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    vj = b.data.vehicle_journeys[2];

    BOOST_CHECK_EQUAL(vj->uri,  "vehicle_journey:vj3");
//    BOOST_CHECK_EQUAL(*vj->validity_pattern,  ValidityPattern(b.begin, "1111111"));
    bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    BOOST_CHECK_EQUAL(*vj->adapted_validity_pattern, *vj->validity_pattern);

    BOOST_CHECK_EQUAL(b.data.vehicle_journeys.size(), 3);
}

BOOST_AUTO_TEST_CASE(impact_network_1){
    navimake::builder b("201303011T1739");
    bg::date end_date = bg::date_from_iso_string("20130308T1739");
    b.generate_dummy_basis();

    VehicleJourney* vj = b.vj("A", "A", "1111111", "", true, "vj1")("stop1", "8:50","9:00")("stop2", "11:00", "11:02").vj;
    //construction du validityPattern du vj1: 1111111
    std::bitset<7> validedays;
    validedays[bt::Sunday] = true;
    validedays[bt::Monday] = true;
    validedays[bt::Tuesday] = true;
    validedays[bt::Wednesday] = true;
    validedays[bt::Thursday] = true;
    validedays[bt::Friday] = true;
    validedays[bt::Saturday] = true;
    vj->validity_pattern->add(vj->validity_pattern->beginning_date, end_date, validedays);
    vj->adapted_validity_pattern->add(vj->adapted_validity_pattern->beginning_date, end_date, validedays);

    vj = b.vj("A", "A", "", "", true, "vj2")("stop1", "10:50","11:00")("stop2","13:00","13:02").vj;
    //construction du validityPattern du vj2: 1110011
    validedays[bt::Sunday] = true;
    validedays[bt::Monday] = false;
    validedays[bt::Tuesday] = false;
    validedays[bt::Wednesday] = true;
    validedays[bt::Thursday] = true;
    validedays[bt::Friday] = true;
    validedays[bt::Saturday] = true;
    vj->validity_pattern->add(vj->validity_pattern->beginning_date, end_date, validedays);
    vj->adapted_validity_pattern->add(vj->adapted_validity_pattern->beginning_date, end_date, validedays);

    vj = b.vj("B", "B", "1111111", "", true, "vj3")("stop1", "10:50","11:00")("stop2","13:00","13:02").vj;
    //construction du validityPattern du vj3: 1111111
    validedays[bt::Sunday] = true;
    validedays[bt::Monday] = true;
    validedays[bt::Tuesday] = true;
    validedays[bt::Wednesday] = true;
    validedays[bt::Thursday] = true;
    validedays[bt::Friday] = true;
    validedays[bt::Saturday] = true;
    vj->validity_pattern->add(vj->validity_pattern->beginning_date, end_date, validedays);
    vj->adapted_validity_pattern->add(vj->adapted_validity_pattern->beginning_date, end_date, validedays);

    b.data.normalize_uri();


    std::map<std::string, std::vector<nt::Message>> messages;
    nt::Message m;
    m.object_type = nt::Type_e::Network;
    m.object_uri = "network:A";
    m.application_period = pt::time_period(pt::time_from_string("2013-03-01 00:00:00"), pt::time_from_string("2013-03-03 23:59:00"));
    m.active_days = nt::Ven;
    m.application_daily_start_hour = pt::duration_from_string("08:30");
    m.application_daily_end_hour = pt::duration_from_string("09:30");

    messages[m.object_uri].push_back(m);

    AtAdaptedLoader loader;
    loader.apply(messages, b.data);

    vj = b.data.vehicle_journeys[0];

    BOOST_CHECK_EQUAL(vj->uri,  "vehicle_journey:vj1");
//    BOOST_CHECK_EQUAL(*vj->validity_pattern,  ValidityPattern(b.begin, "1111111"));
    bg::date testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

//    BOOST_CHECK_EQUAL(*vj->adapted_validity_pattern,  ValidityPattern(b.begin, "1111110"));
    testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    vj = b.data.vehicle_journeys[1];

    BOOST_CHECK_EQUAL(vj->uri,  "vehicle_journey:vj2");
//    BOOST_CHECK_EQUAL(*vj->validity_pattern,  ValidityPattern(b.begin, "1110011"));
    testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

//    BOOST_CHECK_EQUAL(*vj->adapted_validity_pattern,  ValidityPattern(b.begin, "1110011"));
    testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    vj = b.data.vehicle_journeys[2];

    BOOST_CHECK_EQUAL(vj->uri,  "vehicle_journey:vj3");
//    BOOST_CHECK_EQUAL(*vj->validity_pattern,  ValidityPattern(b.begin, "1111111"));
    testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    BOOST_CHECK_EQUAL(*vj->adapted_validity_pattern, *vj->validity_pattern);

    BOOST_CHECK_EQUAL(b.data.vehicle_journeys.size(), 3);
}

BOOST_AUTO_TEST_CASE(impact_network_2){
    navimake::builder b("20130301T1739");
    bg::date end_date = bg::date_from_iso_string("20130308T1739");
    b.generate_dummy_basis();

    VehicleJourney* vj = b.vj("A", "A", "1111111", "", true, "vj1")("stop1", "8:50","9:00")("stop2", "11:00", "11:02").vj;
    //construction du validityPattern du vj1: 1111111
    std::bitset<7> validedays;
    validedays[bt::Sunday] = true;
    validedays[bt::Monday] = true;
    validedays[bt::Tuesday] = true;
    validedays[bt::Wednesday] = true;
    validedays[bt::Thursday] = true;
    validedays[bt::Friday] = true;
    validedays[bt::Saturday] = true;
    vj->validity_pattern->add(vj->validity_pattern->beginning_date, end_date, validedays);
    vj->adapted_validity_pattern->add(vj->adapted_validity_pattern->beginning_date, end_date, validedays);

    vj = b.vj("A", "A", "1110011", "", true, "vj2")("stop1", "10:50","11:00")("stop2","13:00","13:02").vj;
    //construction du validityPattern du vj2: 1110011
    validedays[bt::Sunday] = true;
    validedays[bt::Monday] = false;
    validedays[bt::Tuesday] = false;
    validedays[bt::Wednesday] = true;
    validedays[bt::Thursday] = true;
    validedays[bt::Friday] = true;
    validedays[bt::Saturday] = true;
    vj->validity_pattern->add(vj->validity_pattern->beginning_date, end_date, validedays);
    vj->adapted_validity_pattern->add(vj->adapted_validity_pattern->beginning_date, end_date, validedays);

    vj = b.vj("B", "B", "1111111", "", true, "vj3")("stop1", "10:50","11:00")("stop2","13:00","13:02").vj;
    //construction du validityPattern du vj3: 1111111
    validedays[bt::Sunday] = true;
    validedays[bt::Monday] = true;
    validedays[bt::Tuesday] = true;
    validedays[bt::Wednesday] = true;
    validedays[bt::Thursday] = true;
    validedays[bt::Friday] = true;
    validedays[bt::Saturday] = true;
    vj->validity_pattern->add(vj->validity_pattern->beginning_date, end_date, validedays);
    vj->adapted_validity_pattern->add(vj->adapted_validity_pattern->beginning_date, end_date, validedays);

    b.data.normalize_uri();


    std::map<std::string, std::vector<nt::Message>> messages;
    nt::Message m;
    m.object_type = nt::Type_e::Network;
    m.object_uri = "network:A";
    m.application_period = pt::time_period(pt::time_from_string("2013-03-01 00:00:00"), pt::time_from_string("2013-03-03 23:59:00"));
    m.active_days = nt::Ven;
    m.application_daily_start_hour = pt::duration_from_string("08:30");
    m.application_daily_end_hour = pt::duration_from_string("09:30");

    messages[m.object_uri].push_back(m);

    m.object_type = nt::Type_e::VehicleJourney;
    m.object_uri = "vehicle_journey:vj1";
    m.application_period = pt::time_period(pt::time_from_string("2013-03-01 00:00:00"), pt::time_from_string("2013-03-04 23:59:00"));
    m.active_days = nt::Sam;
    m.application_daily_start_hour = pt::duration_from_string("08:30");
    m.application_daily_end_hour = pt::duration_from_string("09:30");

    messages[m.object_uri].push_back(m);

    AtAdaptedLoader loader;
    loader.apply(messages, b.data);

    vj = b.data.vehicle_journeys[0];

    BOOST_CHECK_EQUAL(vj->uri,  "vehicle_journey:vj1");
//    BOOST_CHECK_EQUAL(*vj->validity_pattern,  ValidityPattern(b.begin, "1111111"));
    bg::date testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

//    BOOST_CHECK_EQUAL(*vj->adapted_validity_pattern,  ValidityPattern(b.begin, "1111100"));
    testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    vj = b.data.vehicle_journeys[1];

    BOOST_CHECK_EQUAL(vj->uri,  "vehicle_journey:vj2");
//    BOOST_CHECK_EQUAL(*vj->validity_pattern,  ValidityPattern(b.begin, "1110011"));
    testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);


//    BOOST_CHECK_EQUAL(*vj->adapted_validity_pattern,  ValidityPattern(b.begin, "1110011"));
    testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    vj = b.data.vehicle_journeys[2];

    BOOST_CHECK_EQUAL(vj->uri,  "vehicle_journey:vj3");
//    BOOST_CHECK_EQUAL(*vj->validity_pattern,  ValidityPattern(b.begin, "1111111"));
    testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    BOOST_CHECK_EQUAL(*vj->adapted_validity_pattern, *vj->validity_pattern);

    BOOST_CHECK_EQUAL(b.data.vehicle_journeys.size(), 3);
}

BOOST_AUTO_TEST_CASE(impact_network_3){
    navimake::builder b("20130301T1739");
    bg::date end_date = bg::date_from_iso_string("20130308T1739");
    b.generate_dummy_basis();

    VehicleJourney* vj = b.vj("A", "A", "1111111", "", true, "vj1")("stop1", "8:50","9:00")("stop2", "11:00", "11:02").vj;
    //construction du validityPattern du vj1: 1111111
    std::bitset<7> validedays;
    validedays[bt::Sunday] = true;
    validedays[bt::Monday] = true;
    validedays[bt::Tuesday] = true;
    validedays[bt::Wednesday] = true;
    validedays[bt::Thursday] = true;
    validedays[bt::Friday] = true;
    validedays[bt::Saturday] = true;
    vj->validity_pattern->add(vj->validity_pattern->beginning_date, end_date, validedays);
    vj->adapted_validity_pattern->add(vj->adapted_validity_pattern->beginning_date, end_date, validedays);

    vj = b.vj("A", "A", "1110011", "", true, "vj2")("stop1", "10:50","11:00")("stop2","13:00","13:02").vj;
    //construction du validityPattern du vj2: 1110011
    validedays[bt::Sunday] = true;
    validedays[bt::Monday] = false;
    validedays[bt::Tuesday] = false;
    validedays[bt::Wednesday] = true;
    validedays[bt::Thursday] = true;
    validedays[bt::Friday] = true;
    validedays[bt::Saturday] = true;
    vj->validity_pattern->add(vj->validity_pattern->beginning_date, end_date, validedays);
    vj->adapted_validity_pattern->add(vj->adapted_validity_pattern->beginning_date, end_date, validedays);

    vj = b.vj("B", "B", "1111111", "", true, "vj3")("stop1", "10:50","11:00")("stop2","13:00","13:02").vj;
    //construction du validityPattern du vj3: 1111111
    validedays[bt::Sunday] = true;
    validedays[bt::Monday] = true;
    validedays[bt::Tuesday] = true;
    validedays[bt::Wednesday] = true;
    validedays[bt::Thursday] = true;
    validedays[bt::Friday] = true;
    validedays[bt::Saturday] = true;
    vj->validity_pattern->add(vj->validity_pattern->beginning_date, end_date, validedays);
    vj->adapted_validity_pattern->add(vj->adapted_validity_pattern->beginning_date, end_date, validedays);

    b.data.normalize_uri();


    std::map<std::string, std::vector<nt::Message>> messages;
    nt::Message m;
    m.object_type = nt::Type_e::Network;
    m.object_uri = "network:A";
    m.application_period = pt::time_period(pt::time_from_string("2013-03-01 00:00:00"), pt::time_from_string("2013-03-03 23:59:00"));
    m.active_days = nt::Ven;
    m.application_daily_start_hour = pt::duration_from_string("08:30");
    m.application_daily_end_hour = pt::duration_from_string("09:30");

    messages[m.object_uri].push_back(m);

    m.object_type = nt::Type_e::VehicleJourney;
    m.object_uri = "vehicle_journey:vj1";
    m.application_period = pt::time_period(pt::time_from_string("2013-03-01 00:00:00"), pt::time_from_string("2013-03-03 23:59:00"));
    m.active_days = nt::Ven;
    m.application_daily_start_hour = pt::duration_from_string("08:30");
    m.application_daily_end_hour = pt::duration_from_string("09:30");

    messages[m.object_uri].push_back(m);

    AtAdaptedLoader loader;
    loader.apply(messages, b.data);

    vj = b.data.vehicle_journeys[0];

    BOOST_CHECK_EQUAL(vj->uri,  "vehicle_journey:vj1");
//    BOOST_CHECK_EQUAL(*vj->validity_pattern,  ValidityPattern(b.begin, "1111111"));
    bg::date testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

//    BOOST_CHECK_EQUAL(*vj->adapted_validity_pattern,  ValidityPattern(b.begin, "1111110"));
    testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    vj = b.data.vehicle_journeys[1];

    BOOST_CHECK_EQUAL(vj->uri,  "vehicle_journey:vj2");
//    BOOST_CHECK_EQUAL(*vj->validity_pattern,  ValidityPattern(b.begin, "1110011"));
    testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

//    BOOST_CHECK_EQUAL(*vj->adapted_validity_pattern,  ValidityPattern(b.begin, "1110011"));
    testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);


    vj = b.data.vehicle_journeys[2];

    BOOST_CHECK_EQUAL(vj->uri,  "vehicle_journey:vj3");
//    BOOST_CHECK_EQUAL(*vj->validity_pattern,  ValidityPattern(b.begin, "1111111"));
    testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    BOOST_CHECK_EQUAL(*vj->adapted_validity_pattern, *vj->validity_pattern);

    BOOST_CHECK_EQUAL(b.data.vehicle_journeys.size(), 3);
}

BOOST_AUTO_TEST_CASE(impact_stoppoint_0){
    navimake::builder b("20130301T1739");
    bg::date end_date = bg::date_from_iso_string("20130308T1739");
    b.generate_dummy_basis();
    VehicleJourney* vj = b.vj("A", "", "", true, "vj1")("stop1", 8000,8050)("stop2", 8200,8250).vj;
    //construction du validityPattern du vj1: 1111111
    std::bitset<7> validedays;
    validedays[bt::Sunday] = true;
    validedays[bt::Monday] = true;
    validedays[bt::Tuesday] = true;
    validedays[bt::Wednesday] = true;
    validedays[bt::Thursday] = true;
    validedays[bt::Friday] = true;
    validedays[bt::Saturday] = true;
    vj->validity_pattern->add(vj->validity_pattern->beginning_date, end_date, validedays);
    vj->adapted_validity_pattern->add(vj->adapted_validity_pattern->beginning_date, end_date, validedays);

    b.data.normalize_uri();


    std::map<std::string, std::vector<nt::Message>> messages;
    nt::Message m;
    m.object_type = nt::Type_e::StopPoint;
    m.object_uri = "stop_point:stop1";
    m.application_period = pt::time_period(pt::time_from_string("2013-03-01 00:00:00"), pt::time_from_string("2013-03-03 23:59:00"));
    m.active_days = nt::Ven | nt::Sam | nt::Dim;
    m.application_daily_start_hour = pt::duration_from_string("00:00");
    m.application_daily_end_hour = pt::duration_from_string("23:59");

    messages[m.object_uri].push_back(m);

    BOOST_CHECK_EQUAL(b.data.stops.size(), 2);

    AtAdaptedLoader loader;
    loader.apply(messages, b.data);

    BOOST_CHECK_EQUAL(b.data.stops.size(), 3);

    vj = b.data.vehicle_journeys[0];
    BOOST_CHECK_EQUAL(vj->uri,  "vehicle_journey:vj1");

//    BOOST_CHECK_EQUAL(*vj->validity_pattern,  ValidityPattern(b.begin, "1111111"));
    bg::date testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

//    BOOST_CHECK_EQUAL(*vj->adapted_validity_pattern,  ValidityPattern(b.begin, "1111110"));
    testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    BOOST_CHECK_EQUAL(vj->stop_time_list.size(), 2);





    vj = b.data.vehicle_journeys[1];
    BOOST_CHECK_EQUAL(vj->uri,  "vehicle_journey:vj1:adapted:1");
    BOOST_CHECK_EQUAL(vj->is_adapted,  true);

    BOOST_CHECK(vj->validity_pattern->days.none());

    testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    BOOST_CHECK_EQUAL(vj->stop_time_list.size(), 1);
    BOOST_CHECK_EQUAL(b.data.vehicle_journeys.size(), 2);
}

//test : message supprimant le vj donc il n'est pas dupliqué
BOOST_AUTO_TEST_CASE(impact_stoppoint_1){
    navimake::builder b("20130301T1739");
    bg::date end_date = bg::date_from_iso_string("20130308T1739");
    b.generate_dummy_basis();
    VehicleJourney* vj = b.vj("A", "", "", true, "vj1")("stop1", 8000,8050)("stop2", 8200,8250).vj;
    //construction du validityPattern du vj1: 1111111
    std::bitset<7> validedays;
    validedays[bt::Sunday] = true;
    validedays[bt::Monday] = true;
    validedays[bt::Tuesday] = true;
    validedays[bt::Wednesday] = true;
    validedays[bt::Thursday] = true;
    validedays[bt::Friday] = true;
    validedays[bt::Saturday] = true;
    vj->validity_pattern->add(vj->validity_pattern->beginning_date, end_date, validedays);
    vj->adapted_validity_pattern->add(vj->adapted_validity_pattern->beginning_date, end_date, validedays);

    b.data.normalize_uri();


    std::map<std::string, std::vector<nt::Message>> messages;
    nt::Message m;
    m.object_type = nt::Type_e::StopPoint;
    m.object_uri = "stop_point:stop1";
    m.application_period = pt::time_period(pt::time_from_string("2013-03-01 00:00:00"), pt::time_from_string("2013-03-03 23:59:00"));
    m.active_days = nt::Ven | nt::Sam | nt::Dim;
    m.application_daily_start_hour = pt::duration_from_string("00:00");
    m.application_daily_end_hour = pt::duration_from_string("23:59");

    messages[m.object_uri].push_back(m);

    m.object_type = nt::Type_e::VehicleJourney;
    m.object_uri = "vehicle_journey:vj1";
    m.application_period = pt::time_period(pt::time_from_string("2013-03-01 00:00:00"), pt::time_from_string("2013-03-03 23:59:00"));
    m.active_days = nt::Ven | nt::Sam | nt::Dim;
    m.application_daily_start_hour = pt::duration_from_string("00:00");
    m.application_daily_end_hour = pt::duration_from_string("23:59");

    messages[m.object_uri].push_back(m);

    BOOST_CHECK_EQUAL(b.data.stops.size(), 2);

    AtAdaptedLoader loader;
    loader.apply(messages, b.data);

    BOOST_CHECK_EQUAL(b.data.stops.size(), 2);

    vj = b.data.vehicle_journeys[0];
    BOOST_CHECK_EQUAL(vj->uri,  "vehicle_journey:vj1");

//    BOOST_CHECK_EQUAL(*vj->validity_pattern,  ValidityPattern(b.begin, "1111111"));
    bg::date testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

//    BOOST_CHECK_EQUAL(*vj->adapted_validity_pattern,  ValidityPattern(b.begin, "1111110"));
    testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    BOOST_CHECK_EQUAL(vj->stop_time_list.size(), 2);
    BOOST_CHECK_EQUAL(b.data.vehicle_journeys.size(), 1);
}

//test 2 messages <> sur un meme vehiclejourney
BOOST_AUTO_TEST_CASE(impact_stoppoint_2){
    navimake::builder b("20130301T1739");
    bg::date end_date = bg::date_from_iso_string("20130308T1739");
    b.generate_dummy_basis();
    VehicleJourney* vj = b.vj("A", "", "", true, "vj1")("stop1", 8000,8050)("stop2", 8200, 8400).vj;
    //construction du validityPattern du vj1: 1111111
    std::bitset<7> validedays;
    validedays[bt::Sunday] = true;
    validedays[bt::Monday] = true;
    validedays[bt::Tuesday] = true;
    validedays[bt::Wednesday] = true;
    validedays[bt::Thursday] = true;
    validedays[bt::Friday] = true;
    validedays[bt::Saturday] = true;
    vj->validity_pattern->add(vj->validity_pattern->beginning_date, end_date, validedays);
    vj->adapted_validity_pattern->add(vj->adapted_validity_pattern->beginning_date, end_date, validedays);

    b.data.normalize_uri();


    std::map<std::string, std::vector<nt::Message>> messages;
    nt::Message m;
    m.object_type = nt::Type_e::StopPoint;
    m.object_uri = "stop_point:stop1";
    m.application_period = pt::time_period(pt::time_from_string("2013-03-01 00:00:00"), pt::time_from_string("2013-03-03 23:59:00"));
    m.active_days = nt::Ven | nt::Sam | nt::Dim;
    m.application_daily_start_hour = pt::duration_from_string("00:00");
    m.application_daily_end_hour = pt::duration_from_string("23:59");

    messages[m.object_uri].push_back(m);

    m.object_type = nt::Type_e::StopPoint;
    m.object_uri = "stop_point:stop2";
    m.application_period = pt::time_period(pt::time_from_string("2013-03-01 00:00:00"), pt::time_from_string("2013-03-03 23:59:00"));
    m.active_days = nt::Ven | nt::Sam | nt::Dim;
    m.application_daily_start_hour = pt::duration_from_string("00:00");
    m.application_daily_end_hour = pt::duration_from_string("23:59");

    messages[m.object_uri].push_back(m);

    BOOST_CHECK_EQUAL(b.data.stops.size(), 2);
    AtAdaptedLoader loader;
    loader.apply(messages, b.data);
    BOOST_CHECK_EQUAL(b.data.stops.size(), 3);

    vj = b.data.vehicle_journeys[0];
    BOOST_CHECK_EQUAL(vj->uri,  "vehicle_journey:vj1");

//    BOOST_CHECK_EQUAL(*vj->validity_pattern,  ValidityPattern(b.begin, "1111111"));
    bg::date testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

//    BOOST_CHECK_EQUAL(*vj->adapted_validity_pattern,  ValidityPattern(b.begin, "1111110"));
    testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    BOOST_CHECK_EQUAL(vj->stop_time_list.size(), 2);


    //le premier vj adapté ne doit pas circulé, il correspond à un état intermédiaire
    vj = b.data.vehicle_journeys[1];
    BOOST_CHECK_EQUAL(vj->uri,  "vehicle_journey:vj1:adapted:1");
    BOOST_CHECK_EQUAL(vj->is_adapted,  true);
    BOOST_CHECK(vj->validity_pattern->days.none());
    BOOST_CHECK(vj->adapted_validity_pattern->days.none());


    vj = b.data.vehicle_journeys[2];
    BOOST_CHECK_EQUAL(vj->uri,  "vehicle_journey:vj1:adapted:2");
    BOOST_CHECK_EQUAL(vj->is_adapted,  true);

    BOOST_CHECK(vj->validity_pattern->days.none());

    testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    BOOST_CHECK_EQUAL(vj->stop_time_list.size(), 0);
    BOOST_CHECK_EQUAL(b.data.vehicle_journeys.size(), 3);
}

BOOST_AUTO_TEST_CASE(impact_stoppoint_passe_minuit){
    navimake::builder b("20130301T1739");
    bg::date end_date = bg::date_from_iso_string("20130308T1739");
    b.generate_dummy_basis();
    VehicleJourney* vj = b.vj("A", "", "", true, "vj1")("stop1", "23:30", "23:40")("stop2", "25:00", "25:10")("stop3", "26:00", "26:10").vj;
    //construction du validityPattern du vj1: 1111111
    std::bitset<7> validedays;
    validedays[bt::Sunday] = true;
    validedays[bt::Monday] = true;
    validedays[bt::Tuesday] = true;
    validedays[bt::Wednesday] = true;
    validedays[bt::Thursday] = true;
    validedays[bt::Friday] = true;
    validedays[bt::Saturday] = true;
    vj->validity_pattern->add(vj->validity_pattern->beginning_date, end_date, validedays);
    vj->adapted_validity_pattern->add(vj->adapted_validity_pattern->beginning_date, end_date, validedays);

    b.data.normalize_uri();


    std::map<std::string, std::vector<nt::Message>> messages;
    nt::Message m;
    m.object_type = nt::Type_e::StopPoint;
    m.object_uri = "stop_point:stop2";
    m.application_period = pt::time_period(pt::time_from_string("2013-03-02 00:00:00"), pt::time_from_string("2013-03-02 23:59:00"));
    m.active_days = nt::Lun | nt::Mar | nt::Mer | nt::Jeu | nt::Ven | nt::Sam | nt::Dim;
    m.application_daily_start_hour = pt::duration_from_string("00:00");
    m.application_daily_end_hour = pt::duration_from_string("23:59");

    messages[m.object_uri].push_back(m);


    AtAdaptedLoader loader;
    loader.apply(messages, b.data);

    vj = b.data.vehicle_journeys[0];
    BOOST_CHECK_EQUAL(vj->uri,  "vehicle_journey:vj1");

//    BOOST_CHECK_EQUAL(*vj->validity_pattern,  ValidityPattern(b.begin, "1111111"));
    bg::date testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

//    BOOST_CHECK_EQUAL(*vj->adapted_validity_pattern,  ValidityPattern(b.begin, "1111110"));
    testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    BOOST_CHECK_EQUAL(vj->stop_time_list.size(), 3);

    vj = b.data.vehicle_journeys[1];
    BOOST_CHECK_EQUAL(vj->uri,  "vehicle_journey:vj1:adapted:1");
    BOOST_CHECK_EQUAL(vj->is_adapted,  true);
    BOOST_CHECK(vj->validity_pattern->days.none());

    testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    BOOST_CHECK_EQUAL(vj->stop_time_list.size(), 2);
    BOOST_CHECK_EQUAL(b.data.vehicle_journeys.size(), 2);
}

//test 2 messages <> sur un meme vehiclejourney, avec des dates d'application qui se chevauche mais qui ne sont pas égales
BOOST_AUTO_TEST_CASE(impact_stoppoint_3){
    navimake::builder b("20130301T1739");
    bg::date end_date = bg::date_from_iso_string("20130308T1739");
    b.generate_dummy_basis();
    VehicleJourney* vj = b.vj("A", "", "", true, "vj1")("stop1", 8000,8050)("stop2", 8200,8250).vj;
    //construction du validityPattern du vj1: 1111111
    std::bitset<7> validedays;
    validedays[bt::Sunday] = true;
    validedays[bt::Monday] = true;
    validedays[bt::Tuesday] = true;
    validedays[bt::Wednesday] = true;
    validedays[bt::Thursday] = true;
    validedays[bt::Friday] = true;
    validedays[bt::Saturday] = true;
    vj->validity_pattern->add(vj->validity_pattern->beginning_date, end_date, validedays);
    vj->adapted_validity_pattern->add(vj->adapted_validity_pattern->beginning_date, end_date, validedays);

    b.data.normalize_uri();


    std::map<std::string, std::vector<nt::Message>> messages;
    nt::Message m;
    m.object_type = nt::Type_e::StopPoint;
    m.object_uri = "stop_point:stop1";
    m.application_period = pt::time_period(pt::time_from_string("2013-03-01 00:00:00"), pt::time_from_string("2013-03-03 23:59:00"));
    m.active_days = nt::Lun | nt::Mar | nt::Mer | nt::Jeu | nt::Ven | nt::Sam | nt::Dim;
    m.application_daily_start_hour = pt::duration_from_string("00:00");
    m.application_daily_end_hour = pt::duration_from_string("23:59");

    messages[m.object_uri].push_back(m);

    m.object_type = nt::Type_e::StopPoint;
    m.object_uri = "stop_point:stop2";
    m.application_period = pt::time_period(pt::time_from_string("2013-03-02 00:00:00"), pt::time_from_string("2013-03-05 23:59:00"));
    m.active_days = nt::Lun | nt::Mar | nt::Mer | nt::Jeu | nt::Ven | nt::Sam | nt::Dim;
    m.application_daily_start_hour = pt::duration_from_string("00:00");
    m.application_daily_end_hour = pt::duration_from_string("23:59");

    messages[m.object_uri].push_back(m);

    AtAdaptedLoader loader;
    loader.apply(messages, b.data);

    BOOST_CHECK_EQUAL(b.data.vehicle_journeys.size(), 4);
    vj = b.data.vehicle_journeys[0];
    BOOST_CHECK_EQUAL(vj->uri,  "vehicle_journey:vj1");

//    BOOST_CHECK_EQUAL(*vj->validity_pattern,  ValidityPattern(b.begin, "1111111"));
    bg::date testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

//    BOOST_CHECK_EQUAL(*vj->adapted_validity_pattern,  ValidityPattern(b.begin, "1111110"));
    testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    BOOST_CHECK_EQUAL(vj->stop_time_list.size(), 2);


    vj = b.data.vehicle_journeys[1];
    BOOST_CHECK_EQUAL(vj->uri,  "vehicle_journey:vj1:adapted:1");
    BOOST_CHECK_EQUAL(vj->is_adapted,  true);
    BOOST_CHECK(vj->validity_pattern->days.none());
    testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    BOOST_CHECK_EQUAL(vj->stop_time_list.size(), 1);
    BOOST_CHECK_EQUAL(vj->stop_time_list.front()->tmp_stop_point->uri, "stop_point:stop2");
    //check stopoint:2



    vj = b.data.vehicle_journeys[2];
    BOOST_CHECK_EQUAL(vj->uri,  "vehicle_journey:vj1:adapted:2");
    BOOST_CHECK_EQUAL(vj->is_adapted,  true);

    BOOST_CHECK(vj->validity_pattern->days.none());

    testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    BOOST_CHECK_EQUAL(vj->stop_time_list.size(), 0);

    vj = b.data.vehicle_journeys[3];
    BOOST_CHECK_EQUAL(vj->uri,  "vehicle_journey:vj1:adapted:3");
    BOOST_CHECK_EQUAL(vj->is_adapted,  true);

    BOOST_CHECK(vj->validity_pattern->days.none());

    testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    BOOST_CHECK_EQUAL(vj->stop_time_list.size(), 1);
    BOOST_CHECK_EQUAL(vj->stop_time_list.front()->tmp_stop_point->uri, "stop_point:stop1");
}

BOOST_AUTO_TEST_CASE(impact_stoppoint_4){
    navimake::builder b("20130301T1739");
    bg::date end_date = bg::date_from_iso_string("20130312T1739");
    b.generate_dummy_basis();
    VehicleJourney* vj = b.vj("A", "", "", true, "vj1")("stop1", 8000,8050)("stop2", 8200,8250).vj;
    //construction du validityPattern du vj1: 1111111
    std::bitset<7> validedays;
    validedays[bt::Sunday] = true;
    validedays[bt::Monday] = true;
    validedays[bt::Tuesday] = true;
    validedays[bt::Wednesday] = true;
    validedays[bt::Thursday] = true;
    validedays[bt::Friday] = true;
    validedays[bt::Saturday] = true;
    vj->validity_pattern->add(vj->validity_pattern->beginning_date, end_date, validedays);
    vj->adapted_validity_pattern->add(vj->adapted_validity_pattern->beginning_date, end_date, validedays);

    b.data.normalize_uri();


    std::map<std::string, std::vector<nt::Message>> messages;
    nt::Message m;
    m.object_type = nt::Type_e::StopPoint;
    m.object_uri = "stop_point:stop1";
    m.application_period = pt::time_period(pt::time_from_string("2013-03-01 00:00:00"), pt::time_from_string("2014-03-01 23:59:00"));
    m.active_days = nt::Sam | nt::Dim | nt::Lun;
    m.application_daily_start_hour = pt::duration_from_string("00:00");
    m.application_daily_end_hour = pt::duration_from_string("23:59");

    messages[m.object_uri].push_back(m);

    m.object_uri = "stop_point:stop2";
    m.application_period = pt::time_period(pt::time_from_string("2013-03-01 00:00:00"), pt::time_from_string("2013-03-15 23:59:00"));
    m.active_days = nt::Ven | nt::Lun;
    messages[m.object_uri].push_back(m);

    BOOST_CHECK_EQUAL(b.data.stops.size(), 2);
    AtAdaptedLoader loader;
    loader.apply(messages, b.data);
    BOOST_CHECK_EQUAL(b.data.stops.size(), 4);

    BOOST_CHECK_EQUAL(b.data.vehicle_journeys.size(), 4);

    vj = b.data.vehicle_journeys[0];
    BOOST_CHECK_EQUAL(vj->uri, "vehicle_journey:vj1");

//    BOOST_CHECK_EQUAL(*vj->validity_pattern,  ValidityPattern(b.begin, "1111111"));
    bg::date testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130308T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130309T1739");
    BOOST_CHECK_EQUAL(vj->validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130308T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130309T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130310T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130311T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->validity_pattern->beginning_date).days()), false);

    BOOST_CHECK_EQUAL(vj->stop_time_list.size(), 2);



    vj = b.data.vehicle_journeys[1];
    BOOST_CHECK_EQUAL(vj->uri,  "vehicle_journey:vj1:adapted:1");
    BOOST_CHECK_EQUAL(vj->is_adapted,  true);

    BOOST_CHECK(vj->validity_pattern->days.none());

    testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130308T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130309T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130310T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130311T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    BOOST_CHECK_EQUAL(vj->stop_time_list.size(), 1);

    vj = b.data.vehicle_journeys[2];
    BOOST_CHECK_EQUAL(vj->uri,  "vehicle_journey:vj1:adapted:2");
    BOOST_CHECK_EQUAL(vj->is_adapted,  true);

    BOOST_CHECK(vj->validity_pattern->days.none());

    testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130308T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130309T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130310T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130311T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    BOOST_CHECK_EQUAL(vj->stop_time_list.size(), 1);

    vj = b.data.vehicle_journeys[3];
    BOOST_CHECK_EQUAL(vj->uri,  "vehicle_journey:vj1:adapted:3");
    BOOST_CHECK_EQUAL(vj->is_adapted,  true);

    BOOST_CHECK(vj->validity_pattern->days.none());

    testdate = bg::date_from_iso_string("20130301T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130302T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130303T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130304T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), true);

    testdate = bg::date_from_iso_string("20130305T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130306T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130307T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130308T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130309T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130310T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), false);

    testdate = bg::date_from_iso_string("20130311T1739");
    BOOST_CHECK_EQUAL(vj->adapted_validity_pattern->check((testdate - vj->adapted_validity_pattern->beginning_date).days()), true);

    BOOST_CHECK_EQUAL(vj->stop_time_list.size(), 0);
}