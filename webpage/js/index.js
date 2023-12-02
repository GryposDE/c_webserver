let timer;
let turn = 0;

let whooo_lr = 1;
let whooo = 0;

function turnMe() {
    if(whooo == 0)
    {
        timer = setInterval(turnFan, 25);
        whooo = 1;
    }
    else
    {
        clearInterval(timer);
        whooo = 0;
        whooo_lr *= -1;
    }
}

function turnFan() {
  let x = document.getElementById("ich");
  turn += (5 * whooo_lr);
  x.style.transform = "rotate("+ (turn % 360) +"deg)"
}