async function get_mcu_status(){
    const response = await fetch("data");
    if (!response.ok) console.error(response.status);
    
    const data = await response.json();
    
    const name = document.getElementById("name");
    const is_active = document.getElementById("is_active");
    
    if (data.is_active === false) {
        is_active.style.color = "red";
    } else {
        is_active.style.color = "green";
    }

    name.innerHTML = data.name;
    is_active.innerHTML = data.is_active;
};

async function send_code() {
    const code_data = document.getElementById("code").value;
    const compiler_errors = document.getElementById("errors");
    const returned_value = document.getElementById("returned");
    
    const code = {
        "code": code_data
    };

    const response = await fetch("data", {
            method: "POST",
            headers: {
                "Content-Type": "application/json"
            },
            body: JSON.stringify(code),
        }
    )

    const resp = await response.json();
    compiler_errors.innerHTML = resp.data;
    returned_value.innerHTML = resp.returned_value;
}

setInterval(get_mcu_status, 1000);
