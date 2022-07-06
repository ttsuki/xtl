/// @file
/// @brief  xtl::cache_map
/// @author (C) 2021-2022 ttsuki

#pragma once
#include "xtl.config.h"

#include <map>
#include <optional>

namespace
XTL_NAMESPACE
{
    template <class TKey, class TValue>
    class cache_map
    {
    public:
        using map_type = std::map<TKey, TValue>;
        using key_type = typename map_type::key_type;
        using value_type = typename map_type::value_type;
        using iterator = typename map_type::iterator;
        using const_iterator = typename map_type::const_iterator;
        using lifetime_type = size_t;
        using lifetime_map_type = std::map<TKey, lifetime_type>;

    private:
        map_type table_;
        lifetime_map_type lifeTable_;
        lifetime_type default_lifetime_;

    public:
        cache_map(lifetime_type default_lifetime = 1)
            : default_lifetime_(default_lifetime)
        {
        }

        auto set_default_lifetime(lifetime_type default_lifetime)
        {
            default_lifetime_ = default_lifetime;
        }

        void age(lifetime_type time = 1)
        {
            for (auto it = lifeTable_.begin(); it != lifeTable_.end();)
            {
                if (it->second < time)
                {
                    table_.erase(it->first);
                    it = lifeTable_.erase(it);
                }
                else
                {
                    it->second -= time;
                    ++it;
                }
            }
        }

        auto empty() const { return table_.empty(); }
        auto size() const { return table_.size(); }
        template <class K> auto count(const K& k) const { return table_.count(k); }

        auto begin() { return table_.begin(); }
        auto end() { return table_.end(); }
        auto begin() const { return table_.begin(); }
        auto end() const { return table_.end(); }

        template <class K> auto find(const K& k) const { return table_.find(k); }

        const TValue& find_or_default(const key_type& k, const TValue& defaultValue = TValue{}) const
        {
            if (auto it = this->find(k); it != this->end())
            {
                return it->second;
            }
            else
            {
                return defaultValue;
            }
        }

        auto clear()
        {
            lifeTable_.clear();
            return table_.clear();
        }

        template <class M = TValue>
        auto insert_or_assign(const key_type& k, M&& obj, std::optional<lifetime_type> lifetime = std::nullopt)
        {
            lifeTable_.insert_or_assign(k, lifetime ? *lifetime : default_lifetime_);
            auto ret = table_.insert_or_assign(k, std::forward<M>(obj));
            return ret;
        }

        void touch(const key_type& k, std::optional<lifetime_type> lifetime = std::nullopt)
        {
            if (auto it = lifeTable_.find(k); it != lifeTable_.end())
            {
                it->second = lifetime ? *lifetime : default_lifetime_;
            }
        }

        void touch(const iterator& position, std::optional<lifetime_type> lifetime = std::nullopt)
        {
            if (auto it = lifeTable_.find(position->first); it != lifeTable_.end())
            {
                it->second = lifetime ? *lifetime : default_lifetime_;
            }
        }

        auto erase(const_iterator& position)
        {
            lifeTable_.erase(position->first);
            return table_.erase(position);
        }

        auto erase(const key_type& k)
        {
            lifeTable_.erase(k);
            return table_.erase(k);
        }
    };
}
