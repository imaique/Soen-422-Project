  // Import the functions you need from the SDKs you need
  import { initializeApp } from "https://www.gstatic.com/firebasejs/10.7.0/firebase-app.js";
  import { getFirestore } from 'https://www.gstatic.com/firebasejs/10.7.0/firebase-firestore.js'

  // TODO: Add SDKs for Firebase products that you want to use
  // https://firebase.google.com/docs/web/setup#available-libraries

  // Your web app's Firebase configuration
  const firebaseConfig = {
    apiKey: "AIzaSyAVBzl3ttDgw-D49Nji3eT6bMGi_Z2plWI",
    authDomain: "soen-422-project.firebaseapp.com",
    databaseURL: "https://soen-422-project-default-rtdb.firebaseio.com",
    projectId: "soen-422-project",
    storageBucket: "soen-422-project.appspot.com",
    messagingSenderId: "507504569682",
    appId: "1:507504569682:web:19b7a862a0d875832b9e8b"
  };
  // Initialize Firebase
const app = initializeApp(firebaseConfig);
export const db = getFirestore(app);