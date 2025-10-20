async function get_mcu_status(){
    const response = await fetch("/data");
    if (!response.ok) console.error(response.status);
    
    const data = await response.json();
    
    const is_active = document.getElementById("is_active");
    const temperature = document.getElementById("temperature");
    
    if (data.is_active === false) {
        is_active.style.color = "red";
    } else {
        is_active.style.color = "green";
    }

    if (data.temperature < 50) {
        temperature.style.color = "blue";
    } else if (data.temperature >= 50 && data.temperature <= 85){
        temperature.style.color = "orange";
    } else {
        temperature.style.color = "red";
    }

    is_active.innerHTML = `${data.is_active}`;
    temperature.innerHTML = `${data.temperature}`;
};

get_mcu_status();
setInterval(get_mcu_status, 1000);
