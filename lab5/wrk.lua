math.randomseed(0)

headers = {["Authorization"] = "Basic bnaW46cGFzc3dvcmQ="} -- login:password
body = ""
users_count = 1000
items_count = 100

request = function()
    path = math.random()
    req = math.random()
    if path < 0.2 then -- /user
        if req < 0.7 then -- get by id
            id = math.random(users_count) - 1
            return wrk.format("GET", string.format("/user?id=%d", id), headers, body)
        else -- put
            phone = math.random(1000, 1000000)
            return wrk.format("PUT", string.format("/user?phone=%d", phone), headers, body)
        end
    elseif path < 0.5 then -- /item
        if req < 0.25 then -- get all
            return wrk.format("GET", "/item/all", {}, body)
        else -- get by id
            id = math.random(items_count) - 1
            return wrk.format("GET", string.format("/item?id=%d", id), {}, body)
        end
    else -- /cart
        if req < 0.2 then -- /add
            id = math.random(items_count) - 1
            return wrk.format("PUT", string.format("/cart/add?item=%d", id), headers, body)
        elseif req < 0.4 then -- /remove
            id = math.random(items_count) - 1
            return wrk.format("PUT", string.format("/cart/remove?item=%d", id), headers, body)
        elseif req < 0.9 then -- get
            return wrk.format("GET", "/cart", headers, body)
        else -- delete
            return wrk.format("DELETE", "/cart", headers, body)
        end
    end
end