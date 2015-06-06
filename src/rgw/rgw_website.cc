#include "common/debug.h"
#include "common/ceph_json.h"

#include "acconfig.h"

#include <errno.h>
#include <string>
#include <list>
#include "include/types.h"
#include "rgw_website.h"

using namespace std;


bool RGWBWRoutingRuleCondition::check_key_condition(const string& key) {
  return (key.size() >= key_prefix_equals.size() &&
          key.compare(0, key_prefix_equals.size(), key_prefix_equals) == 0);
}


void RGWBWRoutingRule::apply_rule(const string& default_protocol, const string& default_hostname,
                                           const string& key, string *new_url)
{
  RGWRedirectInfo& redirect = redirect_info.redirect;

  string protocol = (redirect.protocol.empty() ? redirect.protocol : default_protocol);
  string hostname = (redirect.hostname.empty() ? redirect.hostname : default_hostname);

  *new_url = protocol + "://" + hostname + "/";

  if (!redirect_info.replace_key_prefix_with.empty()) {
    *new_url = redirect_info.replace_key_prefix_with;
    *new_url += key.substr(condition.key_prefix_equals.size());
  } else if (!redirect_info.replace_key_with.empty()) {
    *new_url = redirect_info.replace_key_with;
  } else {
    *new_url = key;
  }
}

bool RGWBWRoutingRules::check_key_condition(const string& key, RGWBWRoutingRule **rule)
{
  for (list<RGWBWRoutingRule>::iterator iter = rules.begin(); iter != rules.end(); ++iter) {
    if (iter->check_key_condition(key)) {
      *rule = &(*iter);
      return true;
    }
  }
  return false;
}

bool RGWBWRoutingRules::check_error_code_condition(int error_code, RGWBWRoutingRule **rule)
{
  for (list<RGWBWRoutingRule>::iterator iter = rules.begin(); iter != rules.end(); ++iter) {
    if (iter->check_error_code_condition(error_code)) {
      *rule = &(*iter);
      return true;
    }
  }
  return false;
}

bool RGWBucketWebsiteConf::should_redirect(const string& key, RGWBWRoutingRule *redirect)
{
  RGWBWRoutingRule *rule;

  if (!routing_rules.check_key_condition(key, &rule)) {
    return false;
  }

  *redirect = *rule;

  return true;
}

void RGWBucketWebsiteConf::get_effective_key(const string& key, string *effective_key)
{

  if (key.empty()) {
    *effective_key = index_doc_suffix;
  } else if (key[key.size() - 1] == '/') {
    *effective_key = key + index_doc_suffix;
  } else {
    *effective_key = key;
  }
}