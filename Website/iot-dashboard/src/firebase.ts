import { initializeApp } from "firebase/app";
import { getFirestore } from "firebase/firestore";

function getEnv(name: string): string {
  const value = import.meta.env[name];
  if (!value) throw new Error(`Missing environment variable: ${name}`);
  return value;
}

const firebaseConfig = {
  apiKey: getEnv("VITE_FIREBASE_API_KEY"),
  authDomain: getEnv("VITE_FIREBASE_AUTH_DOMAIN"),
  projectId: getEnv("VITE_FIREBASE_PROJECT_ID"),
  storageBucket: getEnv("VITE_FIREBASE_STORAGE_BUCKET"),
  messagingSenderId: getEnv("VITE_FIREBASE_MESSAGING_SENDER_ID"),
  appId: getEnv("VITE_FIREBASE_APP_ID"),
};

export const app = initializeApp(firebaseConfig);
export const db = getFirestore(app);
